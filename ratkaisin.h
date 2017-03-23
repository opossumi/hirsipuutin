#pragma once

#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Ratkaisin
{
public:
	Ratkaisin(const std::string &name);

	void play();
private:
	char32_t bestBet(const std::u32string &status,
			const std::unordered_set<std::u32string> &words,
			const std::unordered_set<char32_t> &guesses) const;
	std::u32string guesstimate(const std::u32string &status,
			const std::unordered_set<std::u32string> &words,
			const std::unordered_set<char32_t> &guesses) const;
	void readWords();

	mutable std::ofstream log;
	std::string m_name;
	int m_playedWords;
	int m_totalWords;
	std::unordered_map<int, std::unordered_set<std::u32string> > m_words;
};
