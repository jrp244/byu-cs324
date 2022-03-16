# OpenMP

The purpose of this assignment is to give you hands-on experience with parallel
computation using threads and the OpenMP library.  Code is provided for
computing the Mandelbrot set.  You will parallelize this code using OpenMP and
demonstrate the efficiency by measuring the computation.


# Preparation

Read the following in preparation for this assignment:
 - Sections 1.9.1 - 1.9.2 and 12.6 in the book
 - LLNL's [OpenMP Tutorial](https://hpc-tutorials.llnl.gov/openmp/)
 - Introduction in the header comments of `mandelbrot.c`.

You should do your work on a BYU CS workstation.


# Instructions

The code in `mandelbrot.c` consists of three nested `for` loops, in which
computations are carried out, followed by two nested `for` loops in which the
computations are saved to an image file.  The outer-most loop in which the
computations are made can be parallelized, which will help reduce computation
time when multiple threads are used on multiple cores.  You will use OpenMP to
create this parallelization.  The second loop makes parallelization of the
first possible; if the points were written to the file *as they were
generated*, then all threads would be writing to the file concurrently, and the
file would be out of order.


## Part 1 - Getting Started

Run the following to compile `mandelbrot.c` without compiler optimization
(i.e., without the `-O` option):

```bash
$ gcc -o mandelbrot mandelbrot.c
```

Now execute the command to build the Mandelbrot set with the following:

```bash
$ ./mandelbrot 0.27085 0.27100 0.004640 0.004810 1000 8192 pic.ppm
```

Normalize, and convert the output to png format using `convert` (part of the
ImageMagick suite):

```bash
$ MAGICK_CONFIGURE_PATH=. convert -negate -normalize -fill blue -tint 100 pic.ppm pic.png
```

(Note: setting the `MAGICK_CONFIGURE_PATH` environment variable communicates to
`convert` the directory in which it should look for `policy.xml`, which
contains the maximum resource values.  In particular, by using a custom version
of `policy.xml` in the current directory (`.`), we can override the system-wide
setting 256MiB of memory that `convert` can use to 2GiB.)

Run the following to get the SHA1 sum of the file:

```bash
$ sha1sum pic.ppm
```

Finally, open `pic.png` in a Web browser or image viewer to see it.


## Part 2 - Parallelize Using OpenMP

Use OpenMP to parallelize the outer `for` loop in which the computations are
made.  You will need to apply the proper directives to make it compile and work
properly.  See the
[OpenMP Tutorial](https://hpc-tutorials.llnl.gov/openmp/) for more information
on the directives that should be used.  Remember to use the `-fopenmp` option
with `gcc` to get it to compile properly.

Now use the `OMP_NUM_THREADS` environment variable to run your execution with
just 1 thread, then 2 threads.  In both cases, make sure the SHA1 sum of the
file is the same as that you computed in the previous section.  


## Part 3 - Measure and Record Compute Times

Use the `omp_get_wtick()` function to measure the "wall clock" time required to
carry out the first `for` loop (i.e., the one that is parallelized).  When the
`for` loop has finished, print out the number of seconds required to compute
the loop (in seconds).  Re-compile, and make sure that it runs properly.

Now use the `time` command to run the program again, so in addition to the
elapsed time for the parallelized part of the program, which is printed by
your program, `time` gives you the *total* elapsed time.

```bash
$ OMP_NUM_THREADS=1 time ./mandelbrot 0.27085 0.27100 0.004640 0.004810 1000 8192 pic.ppm
```

Now execute your code for the following numbers of threads
 - 1
 - 2
 - 4
 - 8
 - 16
 - 32

While your command is running, run the `top` command in another window *on the
same machine* to see that your program is not competing with any others for
time on the CPU.


## Part 4 - Questions

 1. How many cores are available for computation on your machine?  Hint: run:
    ```bash
    $ cat /proc/cpuinfo | grep ^proc | wc -l
    ```

 2. What happens to the time associated with computation of the parallel region
    (i.e., the first `for` loop) as the computation time doubles?

 3. At what point (if any) did you observe stop observing the exepected
    performance gain in the parallel region of the code?

 4. At the point you indicated in #3, what was the reason for the lack of
    additional performance gain?

 5. Using the "elapsed time" output by `time`, calculate the overall *speedup*
    (<code>S<sub>p</sub></code>) achieved when four cores were used (i.e., `p` = 4).  Show
    the steps you used to calculate it.

 6. Using the result from #5, compute the efficiency (<code>E<sub>p</sub></code>) of using
    four cores (i.e., `p` = 4)?  Show the steps you used to calculate it.

 7. Briefly explain why the efficiency calculated in #6 is less than 1.

 8. Consider Amdahl's Law:

    <code>
    T<sub>α</sub> = pT/α + (1-p)T
    </code>

    In this case `α` is speedup of the parallel region only, and `p` is the
    fraction of original run time that is parallelizable.

    Find the fraction of parallelizable code, `p`, by using:
    - the answer to #2 to find `α` (speedup of parallelizable code);
    - the "elapsed time" output by `time` for 4 threads as <code>T<sub>α</sub></code>; and
    - the "elapsed time" output by `time` for 1 threads as `T`.


# Cleanup

Remove `pic.ppm` and `pic.png` from your filesystem:

```bash
$ rm -f pic.ppm pic.png
```


# Submission

In comments at the top of `mandelbrot.c`, please include the answers to the
questions from Part 4.

Upload `mandelbrot.c` to the assignment page on LearningSuite.
