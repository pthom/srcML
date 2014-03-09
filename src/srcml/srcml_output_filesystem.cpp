/**
 * @file srcml_output_filesystem.cpp
 *
 * @copyright @copyright Copyright (C) 2014 SDML (www.srcML.org)
 *
 * This file is part of the srcML Toolkit.
 *
 * The srcML Toolkit is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The srcML Toolkit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the srcML Toolkit; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <srcml_output_filesystem.hpp>
#include <srcml.h>
#include <iostream>
#include <iomanip>
#include <boost/filesystem.hpp>

void srcml_output_filesystem(srcml_archive* srcml_arch, const std::string& output_dir, int count) {

    // construct the relative directory
    std::string prefix;
    if (output_dir != "." && output_dir != "./")
        prefix = output_dir;

    while (srcml_unit* unit = srcml_read_unit_header(srcml_arch)) {

        // construct the relative directory
        boost::filesystem::path out(prefix);
        out /= srcml_unit_get_filename(unit);

        // create the path
        if (!is_directory(out.parent_path()))
            boost::filesystem::create_directories(out.parent_path());

        // unparse directory to filename
        srcml_unparse_unit_filename(unit, out.c_str());

        // trace
        ++count;
        std::cerr << std::setw(5) << count << ' ' << out.c_str() << '\n';

        srcml_free_unit(unit);
    }
}
