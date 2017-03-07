#!/usr/bin/python3

import sys
import codecs
import random

MAX_MISSES = 8
VERBOSITY = 0


def main():
    from optparse import OptionParser

    usage = "usage: %prog [options] <dictionary> [[--] <solver path> [solver args...]] "
    parser = OptionParser(usage=usage)
    parser.add_option("-c", "--count", type="int", metavar="NUMBER", dest="count",
                      help="Number of game iterations (0 = all in order)", default=1)
    parser.add_option("-s", "--seed", type="int", metavar="NUMBER", dest="seed",
                      help="Randomization seed")
    parser.add_option("-v", action="count", dest="verbosity", default=0,
                      help="Output verbosity")
    parser.add_option("--skip-dictionary",  dest="skip_dictionary", action="store_true",
                      help="Don't send the dictionary to solver")
    parser.add_option("--save-score-data",  dest="save_score_data", action="store_true",
                      help="Save scores per iteration in machine readable format")
    parser.add_option("--server",  dest="server", action="store_true",
                      help="Server mode")

    (opts, args) = parser.parse_args()

    if len(args) < 1:
        parser.print_help()
        return

    dictionary = []
    dictionary_filename = args[0]
    with codecs.open(dictionary_filename, 'r', 'utf-8') as dictionary_file:
        dictionary = [l.strip().lower() for l in dictionary_file if l.strip()]

    if opts.seed is not None:
        random.seed(opts.seed)

    global VERBOSITY
    VERBOSITY = opts.verbosity

    if opts.server:
        import eventlet
        from eventlet import websocket, wsgi

        class WSFile:
            def __init__(self, ws):
                self.ws = ws

            def write(self, bs):
                self.ws.send(bs)

            def readline(self):
                return self.ws.wait()

            def isatty(self):
                return True

            def flush(self):
                pass

        @websocket.WebSocketWSGI
        def handler(ws):
            wsf = WSFile(ws)
            in_file, out_file = wsf, wsf
            handle_client(in_file, out_file, opts, dictionary)

        wsgi.server(eventlet.listen(('', 8090)), handler)
    else:
        in_file, out_file = prepare_io(args[1:])
        handle_client(in_file, out_file, opts, dictionary)


def handle_client(in_file, out_file, opts, dictionary):
    wins = 0
    losses = 0
    spin = 0

    player = read_utf8(in_file, 1)

    if not opts.skip_dictionary:
        send_dictionary(out_file, dictionary)

    if opts.save_score_data:
        import os.path
        index = 0
        while os.path.isfile('%s.%i.data' % (player, index)):
            index += 1
        score_file = open('%s.%i.data' % (player, index), 'w')

    if opts.count == 0:
        words = dictionary
    else:
        words = random.sample(dictionary, opts.count)

    for i, word in enumerate(words):
        complexity = len({letter for letter in word})
        win, hits, misses = play_hangman(word, in_file, out_file)
        if win:
            wins += 1
            spin += 2*complexity + MAX_MISSES - hits - misses
        else:
            losses += 1
            spin += hits

        score = 1000 * spin / (i + 1)

        if win:
            write_win(out_file, hits, misses, score, word)
        else:
            write_lose(out_file, hits, misses, score, word)

        if opts.save_score_data:
            score_file.write('%i %i %i\n' % (wins, losses, score))

    log(0, 'Won: %i; Lost: %i; Score: %i' % (wins, losses, score))


def masked_word(word, guesses):
    return ''.join(c if c in guesses else '.' for c in word)


def play_hangman(word, in_file, out_file):
    guesses = []
    hits = 0
    misses = 0

    while not all(c in guesses for c in word):
        write_utf8(out_file, masked_word(word, guesses))
        line = read_utf8(in_file)
        guesses.append(line)
        if len(line) == 1:
            if line not in word:
                misses += 1
                write_miss(out_file, hits, misses, line)
            else:
                hits += 1
                write_hit(out_file, hits, misses, line)

        else:
            if line == word:
                hits += 1
                write_hit(out_file, hits, misses, line)
                break
            else:
                misses += 1
                write_miss(out_file, hits, misses, line)

        if misses >= MAX_MISSES:
            return False, hits, misses

    return True, hits, misses


def prepare_io(command):
    if command:
        import subprocess
        proc = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        return (proc.stdout, proc.stdin)
    else:
        return (sys.stdin, sys.stdout)


# Protocol support functions

def log(level, string):
    if level <= VERBOSITY:
        print(string)


def write_utf8(f, string, level=1):
    line = '%s\n' % string
    if f.isatty():
        f.write(line)
    else:
        log(level, '> %s' % string)
        f.write(line.encode('utf-8'))
    f.flush()


def read_utf8(f, level=1):
    line = f.readline() or ""
    if f.isatty():
        return line.strip()
    else:
        decoded = line.decode('utf-8').strip()
        log(level, '< %s' % decoded)
        return decoded


def write_hit(f, hits, misses, guess):
    write_utf8(f, 'HIT %i/%i %s' % (hits, misses, guess))


def write_miss(f, hits, misses, guess):
    write_utf8(f, 'MISS %i/%i %s' % (hits, misses, guess))


def write_win(f, hits, misses, score, word):
    write_utf8(f, 'WIN %i/%i/%i %s' % (hits, misses, score, word))


def write_lose(f, hits, misses, score, word):
    write_utf8(f, 'LOSE %i/%i/%i %s' % (hits, misses, score, word))


def send_dictionary(out_file, dictionary):
    for word in dictionary:
        write_utf8(out_file, word, level=2)
    write_utf8(out_file, '', level=2)
    out_file.flush()


# If run directly
if __name__ == '__main__':
    main()
