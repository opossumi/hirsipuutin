// g++ -std=c++11 *.cpp -o ratkaisin -O2 -DNOLOG

#include "ratkaisin.h"

#include <algorithm>
#include <cmath>
#include <codecvt>
#include <cstring>
#include <iostream>
#include <locale>
#include <sstream>
#include <unordered_map>
#include <vector>

using namespace std;

wstring_convert<codecvt_utf8<char32_t>, char32_t> conv;

u32string to32(const string &str)
{
	return conv.from_bytes(str);
}

string to8(const u32string &str)
{
	return conv.to_bytes(str);
}

string to8(char32_t c)
{
	return to8(u32string()+c);
}

void getline32(istream &in, u32string &str)
{
	string tmp;
	getline(in, tmp);
	str = to32(tmp);
}

vector<string> split(const string &str, char delim, bool skip_empty = false)
{
	vector<string> ret;

	stringstream ss(str);
	string tmp;
	while (getline(ss, tmp, delim)) {
		if (skip_empty && tmp.size() == 0)
			continue;
		ret.push_back(tmp);
	}
	return ret;
}

ostream &operator<<(ostream &os, const u32string &s) {
	return os << to8(s);
}

ostream &operator<<(ostream &os, const char32_t &c) {
	return os << to8(c);
}

Ratkaisin::Ratkaisin(const string &name)
#ifdef NOLOG
	: log("/dev/null"),
#else
	: log("log.txt"),
#endif
	m_name(name),
	m_playedWords(0),
	m_totalWords(0)
{
}

int complexity(u32string word)
{
	unordered_set<char32_t> letters;
	for (auto l : word)
		letters.insert(l);
	return letters.size();
}

void Ratkaisin::play()
{
	// Kerro pelaajan nimi peluuttimelle
	cout << m_name << endl;

	readWords();

	u32string status;

	vector<int> scores;
	scores.reserve(m_totalWords);

	while (true) {		
		getline32(cin, status);
		if (!status.size()) break;
		log << "S: " << status << endl;
		unordered_set<char32_t> guesses, hits, misses;
		unordered_set<u32string> words = m_words[status.size()];

		while (true) {
			log << "Words: " << words.size() << endl;
			if (words.size() < 10) {
				for (auto word : words) {
					log << word << " ";
				}
				log << endl;
			}

			// Make a guess
			//TODO: estimate how many words are left after the guess
			u32string guess = guesstimate(status, words, guesses);
			log << "Guessing " << guess << endl;
			cout << guess << endl;

			if (guess.size() == 1) {
				char32_t g = guess[0];
				if (guesses.find(g) != guesses.end()) {
					log << "CRIT: guessed an already guessed letter" << endl;
				}
				guesses.insert(g);
			}

			u32string result;
			getline32(cin, result);
			getline32(cin, status);

			log << "R: " << result << endl;
			log << "S: " << status << endl;

			if (status.size() == 0 || status[0] == 'L' || status[0] == 'W') {
				vector<string> args = split(to8(status), ' ');
				u32string word = to32(args[2]);

				vector<string> score = split(args[1], '/');
				int hits = atoi(score[0].c_str());
				int misses = atoi(score[1].c_str());
				// score[0] - hits
				// score[1] - misses
				// score[2] - score
				int expected = hits;
				if (args[0] == "WIN") {
#define MAX_MISSES 8
					expected = 2*complexity(word) + MAX_MISSES - hits - misses;
				}
				scores.push_back(expected);
				
				// Calculate total score and compare
				int sum = 0;
				for (int score : scores) {
					sum += score;
				}
				int total = 1000.0 * sum / (double)scores.size();
				log << "Score: " << total << " " << score[2] << endl;

				m_words[word.size()].erase(word);
				++m_playedWords;
				break;
			}

			if (guess.size() == 1 && result[0] == 'H') {
				hits.insert(guess[0]);
				for (auto iter = words.begin(); iter != words.end(); ) {
					bool remove = false;
					for (int i=0; i<status.size(); i++) {
						if (status[i] == '.' && iter->at(i) == guess[0]) {
							remove = true;
							break;
						}
						if (status[i] == guess[0] && iter->at(i) != guess[0]) {
							remove = true;
							break;
						}
					}
					if (remove) {
						iter = words.erase(iter);
					} else {
						iter++;
					}
				}
			} else if (guess.size() == 1 && result[0] == 'M') {
				misses.insert(guess[0]);
				for (auto iter = words.begin(); iter != words.end(); ) {
					bool remove = false;
					for (auto l : *iter) {
						if (l == guess[0]) {
							remove = true;
							break;
						}
					}
					if (remove) {
						iter = words.erase(iter);
					} else {
						iter++;
					}
				}
			}

			if (guess.size() > 1)
				words.erase(guess);
		}
	}
}

char32_t Ratkaisin::bestBet(const u32string &status,
		const unordered_set<u32string> &words,
		const unordered_set<char32_t> &guesses) const
{
	vector<pair<double, char32_t>> choices;

	bool first = true;
	for (auto l : status) {
		if (l != '.') {
			first = false;
			break;
		}
	}

	unordered_map<char32_t, int> letters;

	for (auto word : words) {
		for (int i=0; i<status.size(); i++) {
			if (status[i] == '.') {
				letters[word[i]]++;
			}
		}
	}

	for (auto l : letters) {
		// For each possible word...
		char32_t guess = l.first;
		double lscore = 0;

		for (auto hidden : words) {
			// f was in hidden...
			bool hit = false;
			u32string status2 = status;
			size_t pos = -1;
			while ((pos = hidden.find_first_of(guess, pos+1)) != string::npos) {
				hit = true;
				status2[pos] = l.first;
			}

			// Calc removed words and rate the guess
			int score = 0;
			if (hit) {
				//++hits;
				for (auto word : words) {
					bool remove = false;
					for (int i=0; i<status2.size(); i++) {
						if (status2[i] == '.' && word[i] == guess) {
							remove = true;
							break;
						}
						if (status2[i] == guess && word[i] != guess) {
							remove = true;
							break;
						}
					}
					if (remove) {
						score++;
					}
				}
			} else {
				//++misses;
				for (auto word : words) {
					bool remove = false;
					for (auto l : word) {
						if (l == guess) {
							remove = true;
							break;
						}
					}
					if (remove) {
						score++;
					}
				}
			}
			lscore += score;
		}

		choices.push_back(pair<double, char32_t>(lscore, guess));
	}
	sort(choices.begin(), choices.end(), greater<pair<double, char32_t> >());
	
	return choices[0].second;
}

u32string Ratkaisin::guesstimate(const u32string &status,
		const unordered_set<u32string> &words,
		const unordered_set<char32_t> &guesses) const
{
	u32string guess;
	if (words.size() <= 3) {
		guess = *words.begin();
	} else {
		// Hattuvakio 64
		if (words.size() > 64) {
			unordered_map<char32_t, int> letters;

			for (auto word : words) {
				unordered_set<char32_t> wordLetters;
				for (int i=0; i<status.size(); i++) {
					if (status[i] == '.') {
						letters[word[i]]++;
					}
				}
			}
			int best = 0;
			char32_t choice = 'a';
			for (auto l : letters) {
				if (l.second > best) {
					best = l.second;
					choice = l.first;
				}
			}
			return u32string() + choice;
		}

		guess = bestBet(status, words, guesses);
	}
	return guess;
}

void Ratkaisin::readWords()
{
	u32string word;
	getline32(cin, word);
	while (word.size()) {
		++m_totalWords;
		m_words[word.size()].insert(word);
		getline32(cin, word);
	}
}
