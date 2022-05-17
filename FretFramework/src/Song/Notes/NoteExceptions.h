#pragma once
#include <stdexcept>
#include <string>

class EndofLineException : public std::runtime_error
{
public:
	EndofLineException() : std::runtime_error("reached end of line before full note parse") {}
};

class EndofEventException : public std::runtime_error
{
public:
	EndofEventException() : std::runtime_error("reached end of event before full note parse") {}
};

class InvalidNoteException : public std::runtime_error
{
public:
	InvalidNoteException(int color) : std::runtime_error("invalid color value (" + std::to_string(color) + ')') {}
	InvalidNoteException() : std::runtime_error("invalid color values") {}
};
