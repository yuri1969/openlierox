/*
	Autocompletion support

	OpenLieroX

	code under LGPL
	created on 12-05-2009 by Albert Zeyer
*/

#ifndef __OLX__AUTOCOMPLETION_H__
#define __OLX__AUTOCOMPLETION_H__

#include <string>
#include "Mutex.h"
#include "PreInitVar.h"
#include "Condition.h"
#include "Event.h"
#include "Debug.h"

class AutocompletionInfo {
public:
	struct InputState {
		std::string text;
		size_t pos;
		InputState() : pos(0) {}
		InputState(const std::string& t, size_t p) : text(t), pos(p) {}
		bool operator==(const InputState& s) const { return text == s.text && pos == s.pos; }
		bool operator!=(const InputState& s) const { return !(*this == s); }
	};
	
private:
	Mutex mutex;
	Condition cond;
	PIVar(bool,false) isSet;
	PIVar(bool,false) fail;
	InputState old;
	InputState replace;

public:

	// You are supposed to call setReplace and in the end, call finalize.

	void setReplace(const InputState& old, const InputState& replace) {
		Mutex::ScopedLock lock(mutex);
		isSet = true;
		fail = false;
		this->old = old;
		this->replace = replace;
	}

	void finalize() {
		Mutex::ScopedLock lock(mutex);
		if(!isSet) fail = true;
		isSet = true;
		cond.signal();
	}
	
private:
	bool pop__unsafe(const InputState& old, InputState& replace, bool& fail) {
		if(!isSet) return false;
		isSet = false;
		if(this->fail) {
			fail = true;
			return true;
		}
		if(old != this->old) {
			fail = true;
			return true;
		}
		fail = false;
		replace = this->replace;
		return true;
	}

public:
	void popWait(const InputState& old, InputState& replace, bool& fail) {
		Mutex::ScopedLock lock(mutex);
		while(!isSet) cond.wait(mutex);
		pop__unsafe(old, replace, fail);
	}

	bool pop(const InputState& old, InputState& replace, bool& fail) {
		Mutex::ScopedLock lock(mutex);
		return pop__unsafe(old, replace, fail);
	}

	bool haveSet() {
		Mutex::ScopedLock lock(mutex);
		return isSet;
	}

};


#endif
