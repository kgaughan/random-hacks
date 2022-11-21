#!/usr/bin/env python

import csv
import sys
from os import path
import random
import ConfigParser

# Races *not* including the auction race.
RACES = 8

def read_config(file, section, defaults):
    config = ConfigParser.SafeConfigParser()
    fp = open(file)
    result = {}
    try:
        config.readfp(fp)
        for k, d in defaults.iteritems():
            result[k] = config.get(section, k) if config.has_option(section, k) else d
    finally:
        fp.close()
    return result

def read_csv(file, headings):
    fp = open(file)
    result = []
    try:
        reader = csv.reader(fp)
        for row in reader:
            contents = {heading: row[i] for i, heading in enumerate(headings)}
            result.append(contents)
    finally:
        fp.close()
    return result

if len(sys.argv) <= 1:
    print >> sys.stderr, "No racenight specified."
    print >> sys.stderr, "Syntax: %s <racenight>" % (path.basename(sys.argv[0]),)
    sys.exit(1)

if not path.isfile(sys.argv[1]):
    print >> sys.stderr, "The racenight '%s' does not exist"
    sys.exit(1)

config = read_config(sys.argv[1], "About", {
    "organiser": "We",
    "name": "Racenight",
    "date": "Today",
    "location": "J. Random Pub",
    "horses": "horses.csv",
    "sponsors": "sponsors.csv"})

base = path.dirname(path.abspath(sys.argv[1]))
for k in ['horses', 'sponsors']:
    config[k] = path.join(base, config[k])

horses = read_csv(config["horses"], ["horse", "jockey", "owner"])
random.shuffle(horses)

sponsors = read_csv(config["sponsors"], ["sponsor", "racename"])

print '''
\\documentclass[11pt,a5paper]{article}
\\usepackage[official]{eurosym}
\\usepackage[top=1cm,bottom=1cm,left=0.2cm,right=0.2cm,nohead,nofoot]{geometry} 

\\newcommand{\\hdr}[1]{\\newpage\\begin{center}\\Huge\\sc #1\\par\\vspace{1em}\\end{center}}
\\newcommand{\\spns}[1]{\\begin{center}{\\Large #1}\\end{center}\\vspace{0.25em}}
\\newcounter{race}
\\setcounter{race}{0}
\\newcounter{horse}
\\newcommand{\\racehdr}[3]{%
    \\addtocounter{race}{1}\\ifodd\\therace\\newpage\\fi%
    \\begin{center}{\\Huge\\sc Race N\\raisebox{0.13em}{o} #3\\par{\\bf #1}\\par}\\vspace{1em}{\\sc Sponsored by #2\\par}\\vspace{1em}\\end{center}}
\\newcommand{\\auctracehdr}[3]{%
    \\addtocounter{race}{1}\\newpage%
    \\begin{center}{\\Huge\\sc AUCTION RACE\\\\Race N\\raisebox{0.13em}{o} #3\\par{\\bf #1}\\par}\\vspace{1em}{\\sc Sponsored by #2\\par}\\vspace{1em}\\end{center}}

\\newcommand{\\horseracetbl}[3]{\\addtocounter{horse}{1}\\thehorse & #1 & #2 & #3\\\\}
\\newenvironment{racetbl}{%
    \\setcounter{horse}{0}\\let\\horse=\\horseracetbl%
    \\begin{center}\\begingroup\\large%
    \\begin{tabular}[h]{r@{\\hspace{1em}}l@{\\hspace{1em}}l@{\\hspace{1em}}l}%
    &\\bf Horse &\\bf Jockey &\\bf Owner\\\\\\hline%
}{\\end{tabular}\\endgroup\\end{center}\\vspace{1em}}

\\newcommand{\\auctracetblhorse}[1]{\\addtocounter{horse}{1}\\thehorse & #1 & \\\\}
\\newenvironment{auctracetbl}{%
    \\setcounter{horse}{0}\\let\\horse=\\auctracetblhorse%
    \\begin{center}\\begin{large}%
    \\begin{tabular}[h]{r@{\\hspace{1em}}l@{\\hspace{1em}}l}%
    &\\bf Horse &\\bf \\makebox[16em]{Owner}\\\\\\hline%
}{\\end{tabular}\\end{large}\\end{center}}

\\begin{document}
\\begingroup
\\pagestyle{empty}
\\begin{center}
\\vspace*{10em}'''

print '''
{\\Large\\sc %s \\par}
\\vspace{2em}{\\sc present\\par}\\vspace{2em}
{\\LARGE\\bf ``%s''\\par}
\\vspace{2em}{\\sc to be held in\\par}\\vspace{2em}
{\\Large\\sc %s\\par}
\\vspace{2em}{\\sc on %s\\par}
\\end{center}''' % (config["organiser"], config["name"], config["location"], config["date"])

print '''
\\hdr{Details of the Race Night}
\\begin{large}
\\begin{enumerate}
\\item There will be \\emph{eight horses} in each race. Any one of the horses
      can wil the race and no skill is required to pick the winner.
\\item The Master of Ceremonies will announce when betting is open. You
      can then buy your tickets for the horse you have chosen.
\\item Each ticket costs \\euro{}1 each, and you can buy as you wish on as many
      horses as you wish.
\\item The winners will be the tickets bough on the winning horse and these
      will be paid after the race. The amount of winnings is calculated on
      the total amount taken on that particular race.
\\item The programme/race card gives the names of the horses, owners, and
      jockeys. The commentary on the race will be by number only.
\\item The last race is an auction race. Each horse can be purchased
      tonight---by individuals or syndicates---by the highest bidder.
\\end{enumerate}
\\end{large}
\\vspace{2em}'''

print '''
\\begin{Large}
\\noindent %s\\ wish to thank most sincerely all our sponsors, those who
purchased horses, and those who supported our race night.\\par
\\end{Large}''' % (config["organiser"],)

print '''\\hdr{Many Thanks to Our Kind Sponsors}'''
for sponsor in sponsors:
    print '''\\spns{%s}''' % (sponsor["sponsor"],)

# auct_sponsor = sponsors.pop()

extra = (len(sponsors) % RACES)

i = 0
race = 1
even = False
for sponsor in sponsors:
    if i >= len(horses):
        break
    if extra == 0:
        suffix = ""
    elif even:
        suffix = "b"
    else:
        suffix = "a"
    print '''\\racehdr{%s}{%s}{%d%s}''' % (sponsor["racename"], sponsor["sponsor"], race, suffix)
    print '''\\begin{racetbl}'''
    for j in range(8):
        if i >= len(horses):
            break
        if horses[i]["owner"].strip() == '':
            horses[i]["owner"] = horses[i]["jockey"]
        print '''\\horse{%s}{%s}{%s}''' % (horses[i]["horse"], horses[i]["jockey"], horses[i]["owner"])
        i += 1
    print '''\\end{racetbl}'''
    if extra > 0 and even:
        race += 1
        extra -= 1
    elif extra == 0:
        race += 1
    even = not even
if i < len(horses):
    print >> sys.stderr, "Couldn't use all the horses. Horses unused: %d" % (len(horses) - i,)

print '''\\auctracehdr{%s}{%s}{%s}''' % ("The King of Comedy Steeplechase", "Yours Truly", race)
print '''
\\begin{auctracetbl}
\\horse{Belclare Castle}
\\horse{Killasser Queen}
\\horse{Banada Babe}
\\horse{Lislea Lightning}
\\horse{Aclare Avenger}
\\horse{Kilmactigue Capers}
\\horse{Cloonbarry Cause}
\\horse{Claddagh Dream}
\\end{auctracetbl}
\\newpage\\vspace*{1pt}
\\newpage\\vspace*{1pt}

\\endgroup
\\end{document}'''

# vim:et:
