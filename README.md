HPC Performance Anomaly Suite (HPAS)
====================================

This repository holds the anomaly suite presented in the paper "[HPAS: An HPC Performance Anomaly Suite for Reproducing Performance Variations](https://github.com/peaclab/HPAS/blob/master/docs/ates_icpp19.pdf)"[1]. It consists of a set of synthetic anomalies that reproduce common root causes of performance variations in supercomputers:

* CPU contention
* Cache evictions
* Memory bandwidth interference
* Memory intensive processes
* Memory leaks
* Network contention
* I/O metadata server contention
* I/O storage server contention

These anomalies use processes that run in user space; thus do not require any hardware or kernel modification. For installation details please see below.

An earlier subset of the anomalies were used as part of the papers *Tuncer et al., "Online Diagnosis of Performance Variation in HPC Systems Using Machine Learning"*[2] and  *Tuncer et al., "Diagnosing Performance Variations in HPC Applications Using Machine Learning"*[3].

Installation
------------
A brief intro is here, but check INSTALL for more details.
1. Install C/C++ compilers, GNU autotools
    1. Install shmem if you wish to use netoccupy anomaly. OpenMPI typically
       includes a shmem compiler.
2. If checking out from git, generate the configure script by running the
   command `./autogen.sh`. For the latest release tarball, see the releases.
3. `./configure`
    1. Configure has some options that can be explored, such as prefix
       directory, see `./configure --help`
    2. The cache anomalies measure the cache size during this step, so it's
       important to run configure on the node that the anomaly is going to
       execute on.
    3. `LD_LIBRARY_PATH` and `CFLAGS` should indicate the location of `shmem.h` and
       relevant libraries. If using OpenMPI, use `oshcc` to compile (add
       `CC=oshcc` to `configure` arguments) and oshrun to run.
4. `make`
5. `make install`

More Details on Anomalies
-------------------------

The time units are specified in double precision floats, in seconds. Up to
microsecond granularity can be used. Each anomaly has a `--help` option, which
prints detailed information about command line arguments.

For detailed evaluation of anomalies, refer to our paper at ICPP'19, a copy is
included in `docs/ates_icpp19.pdf`.

#### Note for `netoccupy`
The `netoccupy` anomaly assumes that each node has a hostname of the form e.g.,
`nid00020`, such that each node name is `nid` followed by a unique integer. The
relevant line within `src/netoccupy.c` should be changed to support different
node naming conventions.

### Comparison with anomalies from [2] and [3]
In the 2017 ISC paper [3], and the 2019 TPDS paper [2] an earlier version of the anomaly suite was used.
The following table describes how each anomaly corresponds to the ones in the suite.

| Subsystem | HPAS Anomaly | Anomaly from [2] | Anomaly from [3] |
| --------- | ------------ | ---------------- | -----------------|
| CPU       | cpuoccupy    | dial             | dial             |
| Cache     | cachecopy    | dcopy            | dcopy, ddot      |
| Memory    | memleak      | leak             | leak             |
| Memory    | memeater     | memeater         | memeater         |
| Memory    | membw        | N/A              | N/A              |
| Network   | netoccupy    | N/A              | N/A              |
| Network   | N/A          | linkclog         | N/A              |
| I/O       | iobandwidth  | N/A              | N/A              |
| I/O       | iometadata   | N/A              | N/A              |

In [2], `linkclog` was using wrappers around MPI calls to emulate network contention, whereas `netoccupy`
creates actual network contention by sending/receiving many messages, so they are not equivalent.

Usage of Anomalies
------------------

For SLURM systems, the anomalies can be injected into the application with the
following example line (with necessary modifications) in the job submission script:

```
srun --nodelist=$ANOMALOUS_NODE -N 1 -n 1 $PREFIX/bin/hpas cpuoccupy [--options] &
anomaly_pid=$!
```

For anomalies that stress a shared resource between nodes (I/O and network),
`$ANOMALOUS_NODE` should be different from application nodes, and it should be
one of the application nodes if the anomaly is a CPU, cache or memory anomaly.
The I/O anomalies can be executed with higher `-N` (nodes) and `-n` (ranks)
values for more interference, and the network anomaly has to be executed
with `-N 2`.


Contributing
------------

This anomaly suite is not an exhaustive list of anomaly generators for HPC
systems. Therefore, we are open to additions of new anomalies. If you have an
anomaly generator you would like to contribute, please open a pull request/issue
at GitHub, and we can work together to include the anomalies.

For any bugs/problems, please open a pull request if you have a fix, issue
otherwise.

Using HPAS
----------

If you use this anomaly suite for a publication, please cite [1].

Bibtex entry:
```
@inproceedings{ates:2019,
    author = {Emre Ates and Yijia Zhang and Burak Aksar and Jim Brandt and
              Vitus J. Leung and Manuel Egele and Ayse K. Coskun},
    title = {{HPAS}: An {HPC} Performance Anomaly Suite for Reproducing
             Performance Variations},
    booktitle = {48th International Conference on Parallel Processing
                 (ICPP 2019)},
    year = {2019},
}
```

License
-------

HPAS is licensed under the [BSD 3-Clause license](https://github.com/peaclab/HPAS/blob/master/LICENSE).

References
----------

[1] Emre Ates, Yijia Zhang, Burak Aksar, Jim Brandt, Vitus J. Leung, Manuel Egele, and Ayse K. Coskun. HPAS: An HPC Performance Anomaly Suite for Reproducing Performance Variations. In International Conference on Parallel Processing (ICPP), Aug. 2019

[2]  Ozan Tuncer, Emre Ates, Yijia Zhang, Ata Turk, Jim Brandt, Vitus J. Leung, Manuel Egele, and Ayse K. Coskun. Online Diagnosis of Performance Variation in HPC Systems Using Machine Learning, in IEEE Transactions on Parallel and Distributed Systems (TPDS), vol. 30, no. 4, pp. 883-896, April 2019.

[3] Ozan Tuncer, Emre Ates, Yijia Zhang, Ata Turk, Jim Brandt, Vitus Leung, Manuel Egele, and Ayse K. Coskun. Diagnosing Performance Variations in HPC Applications using Machine Learning. In International Supercomputing Conference, ISC-HPC 2017., pp. 355-373, June 2017. Gauss Award.


