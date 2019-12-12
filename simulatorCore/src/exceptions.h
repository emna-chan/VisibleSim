#ifndef EXCEPTIONS_H__
#define EXCEPTIONS_H__

#include <string>

#include "utils.h"
#include "color.h"

namespace BaseSimulator {

class VisibleSimException : public std::exception {
protected:
    std::string m_msg;
public:
    VisibleSimException()
        : m_msg(std::string("An unknown simulator exception has occured\n")) {}
    VisibleSimException(const std::string &msg)
        : m_msg(msg) {}

    virtual const char* what() const throw() override {
        return (m_msg.c_str());
        // + std::string("\n") + std::string(utils::Backtrace(6))).c_str();
    }
};

//<! @brief Exception thrown if an error as occured during parsing
class ParsingException : VisibleSimException {
public:
    ParsingException() : VisibleSimException(std::string("An unknown error occured during configuration file parsing\n.")) {}
    ParsingException(const std::string &reason)
        : VisibleSimException(TermColor::BRed + std::string("error (config): ")
                              + TermColor::Reset + reason) {}

    virtual const char* what() const throw() override {
        return m_msg.c_str();
    }
};

//<! @brief Exception thrown if an error as occured during command line parsing
class CLIParsingError : VisibleSimException {
public:
    CLIParsingError() : VisibleSimException(std::string("An unknown error occured during command line argument parsing\n.")) {}
    CLIParsingError(const std::string &reason)
        : VisibleSimException(TermColor::BRed + std::string("error (CLI): ") + TermColor::Reset
                              + reason) {}

    virtual const char* what() const throw() override {
        return m_msg.c_str();
    }
};

//!< An exception for marking functions as not implemented
class NotImplementedException : public VisibleSimException {
public:
    NotImplementedException()
        : VisibleSimException(TermColor::BRed + std::string("error (VisibleSim): ")
                              + TermColor::Reset
                              + std::string("Feature not yet implemented.")) {}
    NotImplementedException(const std::string &featureName)
        : VisibleSimException(TermColor::BRed + std::string("error (VisibleSim): ")
                              + TermColor::Reset
                              + std::string("Feature not yet implemented: " + featureName)) {}
};

//!< An exception for notifying invalid uses of functions
class InvalidArgumentException : public VisibleSimException {
public:
    InvalidArgumentException()
        : VisibleSimException(TermColor::BRed + std::string("error (VisibleSim): ")
                              + TermColor::Reset
                              + std::string("Invalid argument supplied to function")) {}
    InvalidArgumentException(const std::string &function_name)
        : VisibleSimException(TermColor::BRed + std::string("error (VisibleSim): ")
                              + TermColor::Reset
                              + std::string("Invalid argument supplied to function: ")
                              + function_name) {}
    InvalidArgumentException(const std::string &function_name, const std::string &arg_name)
        : VisibleSimException(TermColor::BRed + std::string("error (VisibleSim): ")
                              + TermColor::Reset
                              + std::string("Invalid argument supplied to function: ")
                              + function_name + std::string(" -- arg: ") + arg_name) {}

};

} // namespace BaseSimulator

#endif // EXCEPTIONS_H__
