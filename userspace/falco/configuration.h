/*
Copyright (C) 2016 Draios inc.

This file is part of falco.

falco is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

falco is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with falco.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <yaml-cpp/yaml.h>
#include <string>
#include <vector>
#include <list>
#include <iostream>

#include "falco_outputs.h"

class yaml_configuration
{
public:
	std::string m_path;
	yaml_configuration(const std::string& path)
	{
		m_path = path;
		YAML::Node config;
		std::vector<falco_outputs::output_config> outputs;
		try
		{
			m_root = YAML::LoadFile(path);
		}
		catch (const YAML::BadFile& ex)
		{
			std::cerr << "Error reading config file (" + path + "): " + ex.what() + "\n";
			throw;
		}
		catch (const YAML::ParserException& ex)
		{
			std::cerr << "Cannot read config file (" + path + "): " + ex.what() + "\n";
			throw;
		}
	}

	/**
	* Get a scalar value defined at the top level of the config
	*/
	template<typename T>
	const T get_scalar(const std::string& key, const T& default_value)
	{
		try
		{
			auto node = m_root[key];
			if (node.IsDefined())
			{
				return node.as<T>();
			}
		} catch (const YAML::BadConversion& ex)
		{
			std::cerr << "Cannot read config file (" + m_path + "): wrong type at key " + key + "\n";
			throw;
		}

		return default_value;
	}

	/**
	 * Set the top-level node identified by key to value
	 */
	template<typename T>
	void set_scalar(const std::string &key, const T& value)
	{
		auto node = m_root;
		if (node.IsDefined())
		{
			node[key] = value;
		}
	}

	/**
	* Get a scalar value defined inside a 2 level nested structure like:
	* file_output:
	*   enabled: true
	*   filename: output_file.txt
	*
	* get_scalar<bool>("file_output", "enabled", false)
	*/
	template<typename T>
	const T get_scalar(const std::string& key, const std::string& subkey, const T& default_value)
	{
		try
		{
			auto node = m_root[key][subkey];
			if (node.IsDefined())
			{
				return node.as<T>();
			}
		}
		catch (const YAML::BadConversion& ex)
		{
			std::cerr << "Cannot read config file (" + m_path + "): wrong type at key " + key + "\n";
			throw;
		}

		return default_value;
	}

	/**
	 * Set the second-level node identified by key[key][subkey] to value.
	 */
	template<typename T>
	void set_scalar(const std::string& key, const std::string& subkey, const T& value)
	{
		auto node = m_root;
		if (node.IsDefined())
		{
			node[key][subkey] = value;
		}
	}

	// called with the last variadic arg (where the sequence is expected to be found)
	template <typename T>
	void get_sequence(T& ret, const std::string& name)
	{
		YAML::Node child_node = m_root[name];
		if(child_node.IsDefined())
		{
			if(child_node.IsSequence())
			{
				for(const YAML::Node& item : child_node)
				{
					ret.insert(ret.end(), item.as<typename T::value_type>());
				}
			}
			else if(child_node.IsScalar())
			{
				ret.insert(ret.end(), child_node.as<typename T::value_type>());
			}
		}
	}

private:
	YAML::Node m_root;
};


class falco_configuration
{
 public:
	falco_configuration();
	virtual ~falco_configuration();

	void init(std::string conf_filename, std::list<std::string> &cmdline_options);
	void init(std::list<std::string> &cmdline_options);

	std::list<std::string> m_rules_filenames;
	bool m_json_output;
	std::vector<falco_outputs::output_config> m_outputs;
	uint32_t m_notifications_rate;
	uint32_t m_notifications_max_burst;

	falco_common::priority_type m_min_priority;
 private:
	void init_cmdline_options(std::list<std::string> &cmdline_options);

	/**
	 * Given a <key>=<value> specifier, set the appropriate option
	 * in the underlying yaml config. <key> can contain '.'
	 * characters for nesting. Currently only 1- or 2- level keys
	 * are supported and only scalar values are supported.
	 */
	void set_cmdline_option(const std::string &spec);

	yaml_configuration* m_config;
};

