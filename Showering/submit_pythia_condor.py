#!/bin/python

import sys
import os
import argparse

parser = argparse.ArgumentParser(description="Submit phythia generation")
parser.add_argument("--lhedir", type=str, required=True, help="events directory")
parser.add_argument('--destdir', type=str, required=True, help="destination dir")
parser.add_argument('--card', type=str, required=True, help="phantom card")
parser.add_argument('--output', type=str, required=True, help="hepmc/tree filename ")
parser.add_argument('--env', type=str, required=True, help="env file")
parser.add_argument('--exe', type=str, required=True, help="pythia executable")
parser.add_argument('--lhename', type=str, required=True, help="name of lhe file")
parser.add_argument('--nevents', type=str, required=True, help="number of events")
parser.add_argument('--ngens', type=str, required=True, help="number of gens")
args = parser.parse_args()

# Create base dir
if os.path.exists(args.destdir):
    print "Directory exists!" 
    exit()
os.mkdir(args.destdir)

condor = """
executable            = pythia_script.sh
arguments             = $(ProcId) 
output                = output/pythia.$(ClusterId).$(ProcId).out
error                 = error/pythia.$(ClusterId).$(ProcId).err
log                   = log/pythia.$(ClusterId).log
+JobFlavour = "workday"
queue {ngens}
"""

condor = condor.replace("{card}", args.card)
condor = condor.replace("{ngens}", args.ngens)


bash = """#!/bin/bash

GEN=gen$1
LHEFILE={lhedir}/$GEN/{lhename}

echo -e "$GEN >>> Starting";
mkdir $GEN
cd $GEN
source {env}

if [[ ! -f "$LHEFILE" ]]; then
    echo "File not found!"
    exit 1
fi 
cp $LHEFILE .

echo -e "$GEN >>> Pythia is coming...";
{exe} {card} {lhename} {output} {nevents}

echo -e "$GEN >>> Copying results...";
mkdir {destdir}/$GEN
cp {output} {destdir}/$GEN/

echo -e "$GEN >>> End!";
"""

bash = bash.replace("{env}", args.env)
bash = bash.replace("{exe}", args.exe)
bash = bash.replace("{card}", args.card)
bash = bash.replace("{lhename}", args.lhename)
bash = bash.replace("{lhedir}", args.lhedir)
bash = bash.replace("{destdir}", args.destdir)
bash = bash.replace("{output}", args.output)
bash = bash.replace("{nevents}", args.nevents)

with open("condor_job.dat", "w") as fout: 
    fout.write(condor)

with open("pythia_script.sh", "w") as fout: 
    fout.write(bash)
os.system("chmod +x pythia_script.sh")

# Call condor
os.system("condor_submit condor_job.dat")
