# ministat
A small tool to do the statistics legwork on benchmarks etc.

Written by Poul-Henning Kamp, lured into a dark Linux alley and clubbed over the head and hauled away to Github by him, forked by yours truly.

## Build & Install

There should be no dependencies besides the standard libraries and a functional tool-chain.

	$ cd ministat/
	$ make
	$ make PREFIX=/usr install
	install -m 0755 ministat  /usr/bin/ministat

## Usage
The FreeBSD man page is very relevant, pursue it [here](http://www.freebsd.org/cgi/man.cgi?ministat).

	Usage: ministat [-C column] [-c confidence] [-d delimiter(s)] [-ns] [-w width] [file [file ...]]
		confidence = {80%, 90%, 95%, 98%, 99%, 99.5%}
		-C : column number to extract (starts and defaults to 1)
		-d : delimiter(s) string, default to " \t"
		-n : print summary statistics only, no graph/test
		-q : print summary statistics and test only, no graph
		-s : print avg/median/stddev bars on separate lines
		-w : width of graph/test output (default 74 or terminal width)

## Example
From the FreeBSD [man page](http://www.freebsd.org/cgi/man.cgi?ministat)

	$ cat << EOF > iguana
	50
	200
	150
	400
	750
	400
	150
	EOF

	$ cat << EOF > chameleon
	150
	400
	720	
	500
	930
	EOF

	$ ministat -s -w 60 iguana chameleon
	x iguana
	+ chameleon
	+------------------------------------------------------------+
	|x      *  x	     *	       +	   + x	            +|
	| |________M______A_______________|			     |
	| 	      |________________M__A___________________|      |
	+------------------------------------------------------------+
	    N	  Min	     Max     Median	   Avg	     Stddev
	x   7	   50	     750	200	   300	  238.04761
	+   5	  150	     930	500	   540	  299.08193
	No difference proven at 95.0% confidence
## Performance Information
Performance testing is done on a linux server with an Intel(R) Xeon(R) CPU E5-2697 v4 @ 2.30GHz cpu with 6 cores.

The latest version of ministat is *final_version* is the current version which compiles ministat with the -o3 flag giving it a 9% boost in speed over previous versions, *new_strtod* and *integer-mode*. 
All timing data is recorded in this csv file: [link to the csv file](https://github.com/OrenBen-Meir/ministat/blob/master/performance-data/timing.csv)

Below is a timing plot for most ministat versions. 
![](https://raw.githubusercontent.com/OrenBen-Meir/ministat/master/performance-data/graphs/Timing%20Data%20By%20Version.png)

The data is timed using single column files with 4 digit integers. The command used by ministat for timing is:
```
time ministat -q some_file.txt
```
We will break down each version used for ministat, most of which are plotted:

- **Original**: The original unoptimized ministat.
- **Strtok**: New string tokenization. More tests shown it to be slower than the original strtok in a single threaded program in spite of the tag name. However it is thread friendly unlike the original as it can be used without relying on mutexes unlike the original which prevents contention. This gives better performance in a multi-threaded setting.
- **an_qsort**: Replaced qsort with an_qsort for faster sorting. It however is prone to bugs for large file reads. A fix will appear later on. -o2 flag is added for optimization.
- **time_with_dataset_linked_list**: While not plotted, this has a much worse performance. A rolling linked and an option to add verbose timing for certain features is added. However no other feature is added in comparison to the original version. However due to the computation for timing is done regardless of if the timing option is enabled signifigantly reducing performance. You will see that in it's flamegraph, getting timing data consumes almost 95% of the time. The option only allows you to just display already computed timing. This was fixed in *read_parallel* version where timing is only computed if the option is enabled.
- **read_parallel**: Parallelization is added to file reading using raw I/O using c's library for system calls for linux. an_qsort is replaced with regular qsort until a fix can be found. However a boost in performance can be seen. In addition dataset uses a rolling linked list to help with parallelism and an option is made available to show verbose timing information. In addition timing is only computed if the option is enabled. This is a fix from *time_with_dataset_linked_list* Otherwise, it is ignored and certain optimizations are built in so that ministat generally assumes timing is turned off as this feature will only be seldomly used. This is done through biasing branch predictions made by the cpu against computing and displaying timing data. 
- **fix_an_qsort**: Fixed an_qsort on top of parallel file reading with raw I/O. A much bigger performance boost is added as a result.
- **parallel_sort**: Rarallelization is added on top of an_qsort for better performance when sorting. 
- **integer_mode**: Integer mode is added which has boosted performance. Without integer mode, ministat has a similar performance to the *parallel_sort* version. This was achieved by rewriting much of the available functions specifically. However to be maintainable in the long run, using macros similar to an_qsort would be ideal.
- **new_strtod**: Faster version of strtod giving a slight performance boost. 
- **final_version**: -o3 flag added decreasing the timing from previous versions by 9%. This is the current version!

The flamegraphs for each version are created using this command for ministat:
```
ministat -n ./some_large_file.txt ./some_large_file2.txt
```

Here are the links to the flame graphs for each version and how to download with curl on your terminal:
- [original](https://raw.githubusercontent.com/OrenBen-Meir/ministat/master/performance-data/graphs/original.svg)
```
curl https://raw.githubusercontent.com/OrenBen-Meir/ministat/master/performance-data/graphs/original.svg -o original.svg
```
- [Strtok](https://raw.githubusercontent.com/OrenBen-Meir/ministat/master/performance-data/graphs/strtok_faster.svg)
```
curl https://raw.githubusercontent.com/OrenBen-Meir/ministat/master/performance-data/graphs/strtok_faster.svg -o strtok_faster.svg
```
- [an_qsort](https://raw.githubusercontent.com/OrenBen-Meir/ministat/master/performance-data/graphs/an_qsort_original.svg)
```
curl https://raw.githubusercontent.com/OrenBen-Meir/ministat/master/performance-data/graphs/an_qsort_original.svg -o an_qsort_original.svg
```
- [time_with_dataset_linked_list](https://raw.githubusercontent.com/OrenBen-Meir/ministat/master/performance-data/graphs/time_with_dataset_linked_list.svg)
```
curl https://raw.githubusercontent.com/OrenBen-Meir/ministat/master/performance-data/graphs/time_with_dataset_linked_list.svg -o time_with_dataset_linked_list.svg
```
- [read_parallel](https://raw.githubusercontent.com/OrenBen-Meir/ministat/master/performance-data/graphs/read_parallel.svg)
```
curl https://raw.githubusercontent.com/OrenBen-Meir/ministat/master/performance-data/graphs/read_parallel.svg -o read_parallel.svg
```
- [fix_an_qsort](https://raw.githubusercontent.com/OrenBen-Meir/ministat/master/performance-data/graphs/fix_an_qsort.svg)
```
curl https://raw.githubusercontent.com/OrenBen-Meir/ministat/master/performance-data/graphs/fix_an_qsort.svg -o fix_an_qsort.svg
```
- [parallel_sort](https://raw.githubusercontent.com/OrenBen-Meir/ministat/master/performance-data/graphs/parallel_sort.svg)
```
curl https://raw.githubusercontent.com/OrenBen-Meir/ministat/master/performance-data/graphs/parallel_sort.svg -o parallel_sort.svg
```
- [integer_mode (integer mode not enabled)](https://raw.githubusercontent.com/OrenBen-Meir/ministat/master/performance-data/graphs/integer_mode.svg)
```
curl https://raw.githubusercontent.com/OrenBen-Meir/ministat/master/performance-data/graphs/integer_mode.svg -o integer_mode.svg
```
- [integer mode (integer mode enabled)](https://raw.githubusercontent.com/OrenBen-Meir/ministat/master/performance-data/graphs/integer_mode_i_enabled.svg)
```
curl https://raw.githubusercontent.com/OrenBen-Meir/ministat/master/performance-data/graphs/integer_mode_i_enabled.svg -o integer_mode_i_enabled.svg
```
- [new_strtod](https://raw.githubusercontent.com/OrenBen-Meir/ministat/master/performance-data/graphs/new_strtod.svg)
```
curl https://raw.githubusercontent.com/OrenBen-Meir/ministat/master/performance-data/graphs/new_strtod.svg -o new_strtod.svg
```
