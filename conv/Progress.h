#ifndef MSAT_PROGRESS_H
#define MSAT_PROGRESS_H

#include <string>
#include <vector>

namespace msat {

struct ProgressHandler
{
	virtual ~ProgressHandler() {}
	virtual void activity(const std::string& str, int perc = -1, int tot = -1) {}
	virtual void pushTask(const std::string& desc) {}
	virtual void popTask() {}
};

// Singleton class handling progress notification
class Progress
{
	ProgressHandler* handler;

public:
	Progress() : handler(new ProgressHandler) {}
	~Progress();

	static Progress& get();

	// Set the progress handler.  Memory ownership passes to the Progress class
	void setHandler(ProgressHandler* h);

	void activity(const std::string& str, int perc = -1, int tot = -1);

	void pushTask(const std::string& desc);
	void popTask();
};

class ProgressTask
{
	Progress& p;

public:
	ProgressTask(const std::string& desc);
	~ProgressTask();

	void activity(const std::string& str, int perc = -1, int tot = -1);
};


}

// vim:set ts=2 sw=2:
#endif
