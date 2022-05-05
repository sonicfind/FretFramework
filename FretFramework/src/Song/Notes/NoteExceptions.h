#pragma once
#include <stdexcept>
#include <string>

class EndofFileException : public std::runtime_error
{
public:
	EndofFileException() : std::runtime_error("") {}
};

class EndofTrackException : public std::runtime_error
{
public:
	EndofTrackException() : std::runtime_error("") {}
};

class EndofLineException : public std::runtime_error
{
public:
	EndofLineException() : std::runtime_error("") {}
};

class InvalidNoteException : public std::runtime_error
{
public:
	InvalidNoteException(int color) : std::runtime_error("invalid color value (" + std::to_string(color) + ')') {}
	InvalidNoteException() : std::runtime_error("invalid color values") {}
};
