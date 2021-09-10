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

#ifndef SHIRO_SYSTEM_STATISTICS_HH
#define SHIRO_SYSTEM_STATISTICS_HH

#include <cstdint>

namespace shiro::native::system_stats {

    void init();

    // Virtual memory

    uint64_t get_total_memory();

    uint64_t get_memory_usage();

    uint64_t get_process_memory_usage();

    // Physical memory

    uint64_t get_total_physical_memory();

    uint64_t get_physical_memory_usage();

    uint64_t get_physical_process_memory_usage();

    // CPU

    double get_cpu_usage();

    double get_process_cpu_usage();

}

#endif  // SHIRO_SYSTEM_STATISTICS_HH
