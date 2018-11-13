# How to run the shower script

## Install dependencies on lxplus

### Environment settings
First of all setup your environment with recent gcc version on lxplus

` source /afs/cern.ch/sw/lcg/external/gcc/4.9/x86_64-slc6-gcc49-opt/setup.sh `

Create a directory in your home for the needed dependencies, let's call it `{DIR}`

```bash
mkdir {DIR}
cd {DIR}
```

In you `~/.bashrc` set up `$PATH`, `$LD_LIBRARY_PATH` and `$PYTHONPATH` to point the `{DIR}` directory
with the dependencies. Otherwise create a bash script to be sourced with this exports.

**Don't write {DIR} but write down your chosen directory absolute path**

```bash
export PATH={DIR}/libs/bin:$PATH
export LD_LIBRARY_PATH={DIR}/libs/lib:$LD_LIBRARY_PATH
export PYTHONPATH={DIR}/libs/lib64/python2.6/site-packages:$PYTHONPATH
```


### Install LHAPDF 
Let's install a local version of LHAPDF.

```bash
wget http://www.hepforge.org/archive/lhapdf/LHAPDF-6.2.1.tar.gz
tar Jxvf LHAPDF-6.2.1.tar.gz
cd LHAPDF-6.2.1
./configure --prefix={DIR}
make -j4
make install
```

Go to `{DIR}/share/LHAPDF`,  download and unpack in this directory your desidered PDFset from https://lhapdf.hepforge.org/pdfsets.
Usually we use **NNPDF31_nnlo_hessian_pdfas**.


### Install Fastjet
Fastjet is the library used to perform the jet clustering. 

```bash
curl -O http://fastjet.fr/repo/fastjet-3.3.2.tar.gz
tar xf fastjet-3.3.2.tar.gz
cd fastjet-3.3.2/

./configure --prefix={DIR}
make -j4
make check
make install
```

### Install HepMC 
```bash
curl -O http://lcgapp.cern.ch/project/simu/HepMC/download/HepMC-2.06.09.tar.gz
tar xf HepMC-2.06.09.tar.gz
cd HepMC-2.06.09
./configure --prefix={DIR} --with-momentum=GEV --with-length=MM
make -j4 
make install
```


### Install Pythia
Configure Pythia using the configure script. 

```bash
curl -O http://home.thep.lu.se/~torbjorn/pythia8/pythia8235.tgz
tar xf pythia8235.tgz
cd pythia8235

./configure --prefix={DIR}  --with-hepmc3={DIR}  --with-fastjet3={DIR} --with-lhapdf6={DIR} 

make -j4
make install
```

-----------------------------------------------------------------------------------------
## Compile shower script

```bash
g++ shower.cc -o shower.exe `pythia8-config --cxxflags --libs` `root-config --cflags --glibs` `fastjet-config --cxxflags --libs` -lHepMC
```
or just use the script *make.sh*

Remeber to set correctly the variables $PATH, $LD_LIBRARY_PATH as explained at the beginning. 

## Run the shower script

```bash

./shower.exe pythia_conf_file lhe_file output_file n_events
```

