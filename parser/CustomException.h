#pragma once
#include "win_hdrs/win_headers.h"
#include <iostream>
#include <exception>
#include <errno.h>
#include <QtCore>

#define UNKNOWN_EXCEPTION (-1)

class CustomException : public std::exception
{
public:
    CustomException(const QString info, const int32_t code = UNKNOWN_EXCEPTION)
        : std::exception(), m_info(info), m_code(code) {}

    CustomException(const int32_t code)
        : std::exception(), m_info(""),  m_code(code) {}

    virtual ~CustomException() throw () {}

    QString getInfo() { return (m_info.length() > 0) ? m_info : codeToString(); }
    int getCode() { return m_code; }
    virtual const char *what() const throw() { return m_info.toStdString().c_str(); }

protected:
    virtual QString codeToString() { return ""; } /* for inherited classes */
    QString m_info;
    const int m_code;
};

class ParserException : public CustomException
{
public:
    ParserException(const QString info) : CustomException(info) {}
};

