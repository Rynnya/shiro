/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
 * Copyright (C) 2021-2022 Rynnya
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

#ifndef SHIRO_MULTIROW_INSERT_HH
#define SHIRO_MULTIROW_INSERT_HH

#include <sqlpp11/column.h>
#include <sqlpp11/data_types.h>
#include <sqlpp11/mysql/connection.h>

#include <tuple>

namespace sqlpp {

    // Simple implementation for multirow insert
    // Currently this will only work with sqlpp::custom_query and only 1 table

    template <typename... Values>
    struct multirow_insert_object_t {
    public:

        multirow_insert_object_t(Values&&... values)
            : values_(std::make_tuple<Values...>(std::move(values)...))
        {}

        template <typename Context>
        void operator()(Context& context) const {
            context << '(';
            this->serialize(context);
            context << ')';
        }

    private:

        template <size_t It = 0, typename Context>
        void serialize(Context& context) const {
            if constexpr (It != 0) {
                context << ", ";
            }

            sqlpp::serialize(std::get<It>(values_), context);
            if constexpr (It + 1 != sizeof...(Values)) {
                serialize<It + 1>(context);
            }
        }

        std::tuple<Values...> values_;
    };

    template <typename... Columns>
    struct multirow_insert_with_columns {
    public:

        multirow_insert_with_columns(std::tuple<Columns...> columns) : columns_(columns) {
            static_assert(logic::all_t<is_column_t<Columns>::value...>::value, "Columns must have column tag (or be derived from column_t)");
        }

        template <typename... Values>
        multirow_insert_with_columns<Columns...>& set(Values... values) {
            static_assert(sizeof...(Columns) == sizeof...(Values), "amount of values must be same as amount of columns");
            // ?: Is there must be some checks for value types?
            values_.push_back(multirow_insert_object_t<wrap_operand_t<Values>...>(values...));

            return *this;
        }

        size_t size() const noexcept {
            return values_.size();
        }

        template <typename Context>
        void serialize_columns(Context& context) const {
            context << '(';
            serialize_columns_(context);
            context << ')';
        }

        template <typename Context>
        void serialize_values(Context& context) const {
            for (size_t i = 0; i < values_.size(); i++) {
                values_[i](context);

                if (i + 1 != values_.size()) {
                    context << ", ";
                }
            }
        }

    private:

        template <size_t It = 0, typename Context>
        void serialize_columns_(Context& context) const {
            if constexpr (It != 0) {
                context << ", ";
            }

            sqlpp::serialize(std::get<It>(columns_), context);
            if constexpr (It + 1 != sizeof...(Columns)) {
                serialize_columns_<It + 1>(context);
            }
        }

        std::tuple<Columns...> columns_;
        std::vector<multirow_insert_object_t<wrap_operand_t<typename Columns::_traits::_value_type::_cpp_value_type>...>> values_ {};
    };

    template <typename Table>
    struct multirow_insert {
    public:

        multirow_insert(Table table) {
            static_assert(is_table_t<Table>::value, "Table template must have table tag (or be derived from table_t)");
        }

        template <typename... Columns>
        multirow_insert_with_columns<Columns...> with_columns(Columns... columns) {
            static_assert(logic::all_t<is_column_t<Columns>::value...>::value, "all variables must have column tag (or be derived from column_t)");
            static_assert(logic::all_t<std::is_same<Table, typename Columns::_table>::value...>::value, "all variables must be from same table");

            return multirow_insert_with_columns<Columns...>(std::make_tuple(columns...));
        }
    };

    template <typename Context, typename... Columns>
    Context& serialize(const multirow_insert_with_columns<Columns...>& t, Context& context) {

        t.serialize_columns(context);
        context << " VALUES ";
        t.serialize_values(context);

        return context;
    }

}

#endif  // SHIRO_CHANNEL_TABLE_HH
