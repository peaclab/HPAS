HPC Performance Anomaly Suite (HPAS)
====================================

Installation
------------
A brief intro is here, but check INSTALL for more details.
1. Install C/C++ compilers, GNU autotools
    1.1. Install shmem if you wish to use netoccupy anomaly. OpenMPI typically
    includes a shmem compiler.
2. If checking out from git, generate the configure script by running the
   command `./autogen.sh`
3. `./configure`
    3.1. Configure has some options that can be explored, such as prefix
        directory, see `./configure --help`
    3.2. The cache anomalies measure the cache size during this step, so it's
        important to run configure on the node that the anomaly is going to
        execute on.
    3.3. LD_LIBRARY_PATH and CFLAGS should indicate the location of shmem.h and
        relevant libraries. If using OpenMPI, use oshcc to compile (add
        CC=oshcc to `configure` arguments) and oshrun to run.
4. `make`
5. `make install`

More Details on Anomalies
-------------------------

The time units are specified in double precision floats, in seconds. Up to
microsecond granularity can be used. Each anomaly has a `--help` option, which
prints detailed information about command line arguments.

For detailed evaluation of anomalies, refer to our paper at ICPP'19, a copy is
included in `docs/ates_icpp19.pdf`.

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
The I/O anomalies can be executed with higher `-N` and `-n` values for more
interference, and the network anomaly has to be executed with `-N 2`.


Contributing
------------

This anomaly suite is not an exhaustive list of anomaly generators for HPC
systems. Therefore, we are open to additions of new anomalies. If you have an
anomaly generator you would like to contribute, please open a pull request/issue
at GitHub, and we can work together to include the anomalies.

For any bugs/problems, please open a pull request if you have a fix, issue
otherwise.
