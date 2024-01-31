"""
parse_version.py

Parse plugin version and obtain Git commit ID, then generate a C++ header file.
"""

import sys, json, re, os

def parse_json(file_path):
    file_handle = ""
    file_data = ""
    json_data = ""
    try:
        file_handle = open(file_path, "r")
        file_data = file_handle.read()
        json_data = json.loads(file_data)
    except FileNotFoundError:
        sys.stderr.write("Error: JSON file not found: %s\n" % str(file_path))
        quit(1)
    except Exception as e:
        sys.stderr.write("Error: unhandled exception during parsing JSON: %s\n" % e.args[0])
        quit(2)
    finally:
        if 'dpf' not in json_data.keys():
            sys.stderr.write("Error: JSON file is NOT for DPF-based Heavy plugin. Refuse to continue.\n")
            quit(1)
        else:
            return json_data['dpf']

"""
Parse version string.
Version string should be in this format: "MAJOR, MINOR, BUGFIX".
"""
def parse_version(ver_string):
    regex = re.compile(r'\s*(\d+)\s*,\s*(\d+)\s*\s*,\s*(\d+)\s*')
    if regex.match(str(ver_string)):
        ver_nums = regex.findall(ver_string)[0]
        return (ver_nums[0], ver_nums[1], ver_nums[2])
    else:
        sys.stderr.write("WARNING: Invalid version string format. Assume version is (0, 0, 1)!")
        return (0, 0, 1)

def get_git_commit_id():
    git_commit_id = os.popen("git rev-parse HEAD").read()
    if len(git_commit_id) <= 0:
        sys.stderr.write("Error: cannot access Git commit id. Assume unknown.")
        return "unknown"
    else:
        return git_commit_id[0:8]


def write_header_file(output_filename, version_major, version_minor, version_bugfix, git_commit_id):
    template = """
#pragma once

#include <cstdint>

constexpr uint32_t VERSION_MAJOR = {VER_MAJOR};
constexpr uint32_t VERSION_MINOR = {VER_MINOR};
constexpr uint32_t VERSION_BUGFIX = {VER_BUGFIX};

constexpr const char* GIT_COMMIT_ID = "{GIT_COMMIT_ID}";

"""
    result = template.format(VER_MAJOR = version_major, VER_MINOR = version_minor, VER_BUGFIX = version_bugfix, GIT_COMMIT_ID = git_commit_id)

    try:
        file_handle = open(output_filename, "w")
        file_handle.write(result)
    except IOError as e:
        sys.stderr.write("Error: Cannot write file due to IO error: %s\n" % e.args[0])
        quit(1)
    except Exception as e:
        sys.stderr.write("Error: unhandled exception during writing file: %s\n" % e.args[0])
        quit(2)


def print_usage():
    print("Usage: %s <JSON file name> <output header file>" % sys.argv[0])

def main():
    if len(sys.argv) != 3:
        print_usage()
        return
    else:
        json_filename = sys.argv[1]
        output_filename = sys.argv[2]
        plugin_version = ""

        json_data = dict(parse_json(json_filename))
        if "version" in json_data.keys():
            plugin_version = parse_version(json_data['version'])
            # print("Info: got plugin version: %s" % str(plugin_version))
        else:
            sys.stderr.write("WARNING: Version is not defined in JSON. Assume (0, 0, 1)")
            plugin_version = parse_version("0, 0, 1")
        
        git_commit_id = get_git_commit_id()

        write_header_file(output_filename, plugin_version[0], plugin_version[1], plugin_version[2], git_commit_id)

if __name__ == "__main__":
    main()

