#pragma once
#include <boost/archive/tmpdir.hpp>
#include <boost/archive/xml_iarchive.hpp>

#include "joytracer.h"

namespace joytracer
{
    Scene load_scene(const std::string &filename);
} // namespace joytracer
