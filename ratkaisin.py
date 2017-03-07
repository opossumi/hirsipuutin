# Simple hangman solver, guesses letters in order of frequency

print('triviaali')

words = []

word = input().strip()
while word:
    words.append(word)
    word = input().strip()

letters = {letter for word in words for letter in word}
frequencies = [(letter, sum(word.count(letter) for word in words)) for letter in letters]
guess_order = sorted(frequencies, key=lambda a: a[1], reverse=True)

try:
    status = input()
    while status:
        for letter, frequency in guess_order:
            print(letter)
            result = input()
            status = input()
            if status.startswith('WIN') or status.startswith('LOSE') or not status:
                break
except EOFError:
    pass
