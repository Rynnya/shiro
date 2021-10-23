#ifndef SHIRO_CURL_WRAPPER_HH
#define SHIRO_CURL_WRAPPER_HH

#include <cstring>
#include <string>
#include <curl/curl.h>
#include <zlib.h>
#include "../json.hh"

class curl_wrapper
{
public:
    class response
    {
    public:
        response(std::string d, long sc, std::unordered_map<std::string, std::string> heads)
                : data(std::move(d))
                , status_code(sc)
                , headers(std::move(heads))
        {}

        std::string data;
        long status_code;
        std::unordered_map<std::string, std::string> headers;

        nlohmann::json json()
        {
            return nlohmann::json::parse(data);
        }
    };

public:
    curl_wrapper() : m_curl(curl_easy_init())
    {
        //set_option(CURLOPT_VERBOSE, 1L);
        set_option(CURLOPT_SSL_VERIFYPEER, 0L);
    }

    ~curl_wrapper()
    {
        curl_slist_free_all(m_headers);
        curl_easy_cleanup(m_curl);
    }

    response post(const std::string& url, const nlohmann::json& payload, const bool compress = false)
    {
        set_header("Content-Type: application/json");
        return post(url, payload.dump(), compress);
    }

    response post(const std::string& url, const std::string& data, const bool compress = false)
    {
        std::string c_data;

        if (compress)
        {
            std::tuple<bool, std::string> compressed = compress_string(data);
            if (std::get<bool>(compressed))
            {
                c_data = std::get<std::string>(compressed);
                
                set_header("Content-Encoding: gzip");
                const std::string size_header = "Content-Length: " + std::to_string(c_data.size());
                set_header(size_header.c_str());
                set_option(CURLOPT_POSTFIELDS, c_data.c_str());
                set_option(CURLOPT_POSTFIELDSIZE, c_data.size());
            }
            else
            {
                set_option(CURLOPT_POSTFIELDS, data.c_str());
                set_option(CURLOPT_POSTFIELDSIZE, data.size());
            }
        }
        else
        {
            set_option(CURLOPT_POSTFIELDS, data.c_str());
            set_option(CURLOPT_POSTFIELDSIZE, data.size());
        }

        set_option(CURLOPT_URL, url.c_str());
        set_option(CURLOPT_POST, 1);
        set_option(CURLOPT_WRITEFUNCTION, &write_callback);
        set_option(CURLOPT_WRITEDATA, &string_buffer);
        set_option(CURLOPT_HEADERFUNCTION, &header_callback);
        set_option(CURLOPT_HEADERDATA, &headers_buffer);

        auto res = curl_easy_perform(m_curl);

        long status_code;
        curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &status_code);

        if (res != CURLE_OK)
        {
            std::string error_msg = std::string("curl_easy_perform() failed: ") + curl_easy_strerror(res);
            return {std::move(error_msg), status_code, std::move(headers_buffer)};
        }

        return {std::move(string_buffer), status_code, std::move(headers_buffer) };
    }

    template<typename T>
    CURLcode set_option(CURLoption option, T parameter)
    {
        return curl_easy_setopt(m_curl, option, parameter);
    }

    void set_header(const char* header)
    {
        m_headers = curl_slist_append(m_headers, header);
        set_option(CURLOPT_HTTPHEADER, m_headers);
    }

private:
    static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata)
    {
        ((std::string*)userdata)->append(ptr, size * nmemb);
        return size * nmemb;
    }

    static size_t header_callback(char* ptr, size_t size, size_t nmemb, void* userdata)
    {
        const size_t res = size * nmemb;
        const std::string header = std::string(ptr, res);

        const auto& it = header.find(':');
        if (it == std::string::npos)
            return res;

        /*
        * Let me explain a little bit this -> header.substr(it + 2, res - it - 4)
        * 
        * Well, offset to get rid of header name and ':' and ' ' after header
        * And then we remove additional '\r\n' from end, cuz no one really needs this
        */
        ((std::unordered_map<std::string, std::string>*)userdata)->insert({ header.substr(0, it), header.substr(it + 2, res - it - 4) });
        return res;
    }
    /*!
     * @brief gzip compress a string
     *
     * @param[in] str string to compress
     * @return gzip-compressed string
     *
     * @throw std::runtime_error in case of errors
     *
     * @note Code from <https://panthema.net/2007/0328-ZLibString.html>. Adjusted by Niels Lohmann.
     *
     * @copyright Copyright 2007 Timo Bingmann <tb@panthema.net>.
     *            Distributed under the Boost Software License, Version 1.0.
     *            (See http://www.boost.org/LICENSE_1_0.txt)
     */
    std::tuple<bool, std::string> compress_string(const std::string& str)
    {
        std::string result;
        z_stream zs;                        // z_stream is zlib's control structure
        std::memset(&zs, 0, sizeof(zs));

        int ret = deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED, 15 + 16, 9, Z_DEFAULT_STRATEGY);
        if (ret != Z_OK)
        {
            //throw (std::runtime_error("deflateInit2 failed while compressing."));
            return { false, "" };
        }

        // For the compress
        zs.next_in = (Bytef*)str.data();
        zs.avail_in = static_cast<uInt>(str.size());           // set the z_stream's input

        char outbuffer[32768];

        // retrieve the compressed bytes blockwise
        do
        {
            zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
            zs.avail_out = sizeof(outbuffer);

            ret = deflate(&zs, Z_FINISH);

            if (result.size() < zs.total_out)
            {
                // append the block to the output string
                result.append(outbuffer, zs.total_out - result.size());
            }
        }
        while (ret == Z_OK);

        deflateEnd(&zs);

        if (ret != Z_STREAM_END)            // an error occurred that was not EOF
        {
            //std::ostringstream oss;
            //oss << "Exception during zlib compression: (" << ret << ") " << zs.msg;
            //throw (std::runtime_error(oss.str()));
            return { false, "" };
        }

        return { true, result };
    }

private:
    CURL* const m_curl;
    struct curl_slist* m_headers = nullptr;
    std::string string_buffer;
    std::unordered_map<std::string, std::string> headers_buffer;
};

#endif  // SHIRO_CURL_WRAPPER_HH
