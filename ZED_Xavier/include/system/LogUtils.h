#ifndef LOGUTILS_H_
#define LOGUTILS_H_

#include <iostream>
#include <fstream>
#include <string>


class TeeBuf : public std::streambuf {

	public:
		TeeBuf(std::streambuf* bufa, std::streambuf* bufb) : bufa(bufa), bufb(bufb) {}

	private:
		virtual int overflow(int c) {
			if (c == EOF) return !EOF;
			bool eofa = bufa->sputc(c) == EOF;
			bool eofb = bufb->sputc(c) == EOF;
			return (eofa or eofb) ? EOF : c;
		}

		virtual int sync() {
			bool synca = bufa->pubsync() == 0;
			bool syncb = bufb->pubsync() == 0;
			return (synca and syncb) ? 0 : -1;
		}

		std::streambuf* bufa;
		std::streambuf* bufb;
};


class LogStream : public std::ostream {
	public:
		LogStream(std::ostream& stream, std::string filename) : std::ostream(&buf), buf(stream.rdbuf(), fstream.rdbuf()), fstream(filename) {}

	private:
		TeeBuf buf;
		std::ofstream fstream;
};


int mkdir_recursive(const char* pathname, mode_t mode);

#endif
