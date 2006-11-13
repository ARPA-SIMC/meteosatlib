#include "Progress.h"

namespace msat {

static Progress* instance = 0;

Progress::Progress& get()
{
	if (!instance)
		instance = new Progress;
	return *instance;
}

Progress::~Progress()
{
	delete handler;
}

void Progress::setHandler(ProgressHandler* h)
{
	delete handler;
	handler = h;
}

void Progress::activity(const std::string& str, int perc, int tot)
{
	handler->activity(str, perc, tot);
}

void Progress::pushTask(const std::string& desc)
{
	handler->pushTask(desc);
}

void Progress::popTask()
{
	handler->popTask();
}


ProgressTask::ProgressTask(const std::string& desc) : p(Progress::get())
{
	p.pushTask(desc);
}

ProgressTask::~ProgressTask()
{
	p.popTask();
}

void ProgressTask::activity(const std::string& str, int perc, int tot)
{
	p.activity(str, perc, tot);
}

}

// vim:set ts=2 sw=2:
