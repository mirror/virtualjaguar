/*
 * SDLEMU library - Free sdl related functions library
 * Copyrigh(c) 1999-2002 sdlemu development crew
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "sdlemu_config.h"

#include <stdlib.h>								// For atoi()
#include <stdio.h>								// For fopen() and friends
#include <cstring>								// For strcmp()
#include <string>
#include <list>

using namespace std;

class token_list
{
public:
	token_list(const string &name) : m_name(name), m_value(""), m_token("") {}
	void add_token_variable(const string &var) { m_token = var; }
	void add_token_value(const string &value) { m_value = value; }
	const string &LineName() const { return m_name; }
	const string &Token() const { return m_token; }
	const string &Value() const { return m_value; }
private:
	std::string m_name;
	std::string m_value;
	std::string m_token;
};

std::list<token_list> vec;

void string_tokenize_variable()
{
	list<token_list>::iterator p;
	const string delim = " ";
	for(p = vec.begin(); p != vec.end(); p++) {
		string::size_type lastPos = (*p).LineName().find_first_not_of(delim, 0);
		string::size_type pos     = (*p).LineName().find_first_of(delim, lastPos);

		if(string::npos != pos && string::npos != lastPos) {
			string s = (*p).LineName().substr(lastPos, pos - lastPos);
			(*p).add_token_variable(s);
		}
	}
}

void string_tokenize_value()
{
	list<token_list>::iterator p;
	const string delim = " =\n\t\r";		// "\r" needed for Win32 compatibility...

	for(p = vec.begin(); p != vec.end(); p++) {
		string::size_type lastPos = (*p).LineName().find_first_of(delim, 0);
		string::size_type pos     = (*p).LineName().find_first_not_of(delim, lastPos);

		if(string::npos != pos && string::npos != lastPos) {
			string s = (*p).LineName().substr(pos);
			(*p).add_token_value(s);
		}
	}
}

int sdlemu_init_config(const char *filename)
{
	FILE *f = fopen(filename, "r");
	if(!f) return 0;
	
	fseek(f, 0, SEEK_END);
	int len = ftell(f);
	fseek(f, 0, SEEK_SET);
	
	char *s = new char[len];
	fread(s, 1, len, f);
	string str(s);

	const string delim = "\n\r";		// "\r" needed for Win32 compatibility...
	string::size_type lastPos = str.find_first_not_of(delim, 0);
	string::size_type pos     = str.find_first_of(delim, lastPos);

	while (string::npos != pos || string::npos != lastPos) {
		string string = str.substr(lastPos, pos - lastPos);
		if(string[0] == '#')
		{
		}
		else if(string[0] == '[')
		{
		}
		else
		{
			vec.push_back(string);
		}
		lastPos = str.find_first_not_of(delim, pos);
		pos = str.find_first_of(delim, lastPos);
	}
	string_tokenize_variable();
	string_tokenize_value();
	delete [] s;
	fclose(f);
	return 1;
}

const char *sdlemu_getval_string(const char *key_string, const char *default_string)
{
	list<token_list>::iterator p;
	for(p = vec.begin(); p != vec.end(); p++) {
		
		if(strcmp((*p).Token().c_str(), key_string) == 0)
			return (*p).Value().c_str();
	}
	return default_string;
}

int sdlemu_getval_int(const char *key_string, int default_int)
{
	list<token_list>::iterator p;
	for(p = vec.begin(); p != vec.end(); p++) {
		
		if(strcmp((*p).Token().c_str(), key_string) == 0) {
			const char *ret = (*p).Value().c_str();
			if(ret) return atoi(ret);
		}
	}
	return default_int;
}

int sdlemu_getval_bool(const char *key_string, int default_int)
{
	list<token_list>::iterator p;
	for(p = vec.begin(); p != vec.end(); p++) {
		
		if(strcmp((*p).Token().c_str(), key_string) == 0) {
			const char *ret = (*p).Value().c_str();
			if(ret) return atoi(ret)>0;
		}
	}
	return default_int;
}
