#include "Progress.h"

using namespace std;

namespace msat {

static Progress* instance = 0;

Progress& Progress::get()
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

void StreamProgressHandler::outputIndent()
{
	for (int i = 0; i < indent; ++i)
		out << ' ';
}

void StreamProgressHandler::activity(const std::string& str, int perc, int tot)
{
	outputIndent();
	out << str;
	if (perc != -1 && tot != -1)
		out << " (" << perc << "/" << tot << ")";
	else if (perc != -1)
		out << " " << perc << "%";
	out << endl;
}

void StreamProgressHandler::pushTask(const std::string& desc)
{
	outputIndent();
	out << desc << "..." << endl;

	++indent ;
}

void StreamProgressHandler::popTask()
{
	--indent;
}

}

// vim:set ts=2 sw=2:
