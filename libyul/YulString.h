/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * String abstraction that avoids copies.
 */

#pragma once

#include <boost/noncopyable.hpp>

#include <map>
#include <memory>
#include <vector>
#include <string>

namespace dev
{
namespace yul
{

class YulStringRepository: boost::noncopyable
{
public:
	struct Handle {
		size_t id;
		size_t hash;
	};
	YulStringRepository(): m_strings{std::make_pair(zeroHash(), std::make_shared<std::string>())}
	{
		m_handles[std::string{}] = Handle { 0, zeroHash() };
	}
	static YulStringRepository& instance()
	{
		static YulStringRepository inst;
		return inst;
	}
	Handle stringToHandle(std::string const& _string)
	{
		if (_string.empty())
			return { 0, m_strings[0].first };
		// TODO: This should be a hash table lookup using hash(_string) as hash.
		Handle& handle = m_handles[_string];
		if (handle.id == 0)
		{
			handle.hash = hash(_string);
			m_strings.emplace_back(std::make_pair(handle.hash, std::make_shared<std::string>(_string)));
			handle.id = m_strings.size() - 1;
		}
		return handle;
	}
	std::string const& idToString(size_t _id) const
	{
		return *m_strings.at(_id).second;
	}

	static size_t hash(std::string const& v) {
		// FNV hash - can be replaced by a better one, e.g. xxhash64
		std::size_t hash = 14695981039346656037u;
		for(std::string::const_iterator it = v.begin(), end = v.end(); it != end; ++it)
		{
			hash *= 1099511628211u;
			hash ^= *it;
		}

		return hash;
	}
	static size_t zeroHash() {
		static size_t zeroHashValue = hash("");
		return zeroHashValue;
	}
private:
	std::vector<std::pair<size_t, std::shared_ptr<std::string>>> m_strings;
	std::map<std::string, Handle> m_handles;
};

class YulString
{
public:
	YulString() = default;
	explicit YulString(std::string const& _s): m_handle(YulStringRepository::instance().stringToHandle(_s)) {}
	YulString(YulString const&) = default;
	YulString(YulString&&) = default;
	YulString& operator=(YulString const&) = default;
	YulString& operator=(YulString&&) = default;

	/// This is not consistent with the string <-operator!
	bool operator<(YulString const& _other) const
	{
		if (m_handle.hash < _other.m_handle.hash) return true;
		if (_other.m_handle.hash < m_handle.hash) return false;
		if (m_handle.id == _other.m_handle.id) return false;
		return str() < _other.str();
	}
	bool operator==(YulString const& _other) const { return m_handle.id == _other.m_handle.id; }
	bool operator!=(YulString const& _other) const { return m_handle.id != _other.m_handle.id; }

	bool empty() const { return m_handle.id == 0; }
	std::string const& str() const
	{
		return YulStringRepository::instance().idToString(m_handle.id);
	}

private:
	/// ID of the string. Assumes that the empty string has ID zero.
	YulStringRepository::Handle m_handle { 0, YulStringRepository::zeroHash() };
};

}
}
