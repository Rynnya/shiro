/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
 * Copyright (C) 2018-2020 Marc3842h, czapek
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <regex>

#include "../thirdparty/multipartparser.hh"
#include "multipart_parser.hh"
#include "string_utils.hh"

namespace callbacks {

    struct multipart_struct {
        shiro::utils::multipart_form_parts parts = {};
        std::string header_name;
        std::string header_value;
    };

    // forward declarations
    int on_body_begin       (multipartparser* parser);
    int on_part_begin       (multipartparser* parser);
    int on_header_field     (multipartparser* parser, const char* data, size_t size);
    int on_header_value     (multipartparser* parser, const char* data, size_t size);
    int on_headers_complete (multipartparser* parser);
    int on_data             (multipartparser* parser, const char* data, size_t size);
    int on_part_end         (multipartparser* parser);
    int on_body_end         (multipartparser* parser);

    // additional functions
    void on_header_done(multipartparser* parser);

    // real implementation

    int on_body_begin(multipartparser* parser) {
        return 0;
    }

    int on_part_begin(multipartparser* parser) {
        static_cast<multipart_struct*>(parser->data)->parts.emplace_back(shiro::utils::multipart_form_part {});
        return 0;
    }

    int on_header_field(multipartparser* parser, const char* data, size_t size) {
        auto ptr = static_cast<multipart_struct*>(parser->data);
        if (!ptr->header_value.empty()) {
            on_header_done(parser);
        }

        ptr->header_name.append(data, size);
        return 0;
    };


    int on_header_value(multipartparser* parser, const char* data, size_t size) {
        static_cast<multipart_struct*>(parser->data)->header_value.append(data, size);
        return 0;
    };

    int on_headers_complete(multipartparser* parser) {
        if (!static_cast<multipart_struct*>(parser->data)->header_value.empty()) {
            on_header_done(parser);
        }

        return 0;
    };

    int on_data(multipartparser* parser, const char* data, size_t size) {
        static_cast<multipart_struct*>(parser->data)->parts.back().body.append(data, size);
        return 0;
    };

    int on_part_end(multipartparser* parser) {
        return 0;
    };

    int on_body_end(multipartparser* parser) {
        return 0;
    };

    void on_header_done(multipartparser* parser) {
        auto ptr = static_cast<multipart_struct*>(parser->data);
        ptr->parts.back().headers[ptr->header_name] = ptr->header_value;

        ptr->header_name.clear();
        ptr->header_value.clear();
    };
}

shiro::utils::multipart_parser::multipart_parser(const std::string &body, const std::string &content_type)
    : body(body)
    , content_type(content_type) {
    // Initialized in initializer list
}

shiro::utils::multipart_form_parts shiro::utils::multipart_parser::parse() {
    if (this->content_type.find("multipart/form-data") == std::string::npos) {
        return {};
    }

    std::string::size_type boundary_pos = this->content_type.find("boundary=");

    if (boundary_pos == std::string::npos) {
        return {};
    }

    std::string boundary = this->content_type.substr(boundary_pos + 9);

    multipartparser parser {};
    multipartparser_callbacks callbacks {};

    multipartparser_callbacks_init(&callbacks);

    callbacks.on_body_begin =       callbacks::on_body_begin;
    callbacks.on_part_begin =       callbacks::on_part_begin;
    callbacks.on_header_field =     callbacks::on_header_field;
    callbacks.on_header_value =     callbacks::on_header_value;
    callbacks.on_headers_complete = callbacks::on_headers_complete;
    callbacks.on_data =             callbacks::on_data;
    callbacks.on_part_end =         callbacks::on_part_end;
    callbacks.on_body_end =         callbacks::on_body_end;

    multipartparser_init(&parser, boundary.c_str());
    parser.data = static_cast<void*>(new callbacks::multipart_struct {});

    size_t result = multipartparser_execute(&parser, &callbacks, this->body.c_str(), this->body.size());
    multipart_form_parts parts = multipart_form_parts { static_cast<callbacks::multipart_struct*>(parser.data)->parts };
    delete static_cast<callbacks::multipart_struct*>(parser.data);

    if (result != this->body.size()) {
        return {};
    }

    return parts;
}

shiro::utils::multipart_form_fields shiro::utils::multipart_parser::parse_fields() {
    multipart_form_parts parts = this->parse();
    multipart_form_fields fields {};

    for (const multipart_form_part &part : parts) {
        if (part.headers.find("Content-Disposition") == part.headers.end()) {
            continue;
        }

        static const std::regex field_name_regex(".+name=\"(.+)\"");
        static const std::regex field_filename_regex(".+filename=\"(.+)\"");

        std::smatch field_name_regex_match, field_filename_regex_match;

        if (part.headers.find("Content-Type") != part.headers.end()) {
            if (std::regex_match(part.headers.at("Content-Disposition"), field_filename_regex_match, field_filename_regex)) {
                std::string field_name = field_filename_regex_match[1];
                multipart_form_field field;

                // workaround for peppy's gay
                if (field_filename_regex_match[1] == "score")
                    field_name = "replay-bin";

                field.body = part.body;
                field.content_type = part.headers.at("Content-Type");
                field.name = field_name;
                field.type = multipart_field_type::file;

                fields.insert({ field_name, field });
            }
        }
        else {
            if (std::regex_match(part.headers.at("Content-Disposition"), field_name_regex_match, field_name_regex)) {
                multipart_form_field field;

                field.body = part.body;
                field.content_type = "text/plain";
                field.name = field_name_regex_match[1];
                field.type = multipart_field_type::text;

                fields.insert({ field_name_regex_match[1], field });
            }
        }
    }

    return fields;
}
