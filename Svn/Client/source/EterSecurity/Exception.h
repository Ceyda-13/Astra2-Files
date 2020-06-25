#pragma once

#include <vector>
#include <stdexcept>
#include <string>

class Exception : public std::exception
{
public:
	Exception(const std::string &message);

	virtual ~Exception() throw() {}

	std::string message;

	virtual const char* what() const throw() override;

	
	std::vector<std::string> get_stack_trace() const;

	
	std::string get_message_and_stack_trace() const;

private:
	enum { max_frames = 32 };
	mutable void *frames[max_frames];
	int32_t num_frames;
	mutable std::string buffer;
};

