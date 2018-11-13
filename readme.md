# Bruceforcer

# Setup
 * Unzip repository
 * make

# Run
 ./bruceforcer -h 'hash with salt'
 
 Bruceforcer will run normally with 4 threads. You can modify this by
 using the -t <N> parameter.

# optional parameters
  -b	Go straight into bruceforce
  
  -s <N bytes> 
	set max bytesize flag.
	If will stop when hitting this size
  
  -t <N threads> 
	set threads
  
  -n <N> Jump into a wordlength
  
  -d <directorypath/>
	Change directory

# How to works
 Bruceforcer will start searching in the directory folder for files containing possible passwords. It will then try to recreate these passwords with given salt from the -h parameter. If a password is found, you will be prompted with it at the end. If you want, you can continue with the bruteforce process. While processing, you will be displayed with a progress. And in the bruteforce process, there are some details like the wpm number, words per minute which you can use to calculate the time it will take to finnish a set with passwords. b> means what the threads wordlength is at. 

 The application will start iterating by default at length 2, and work its way up to
longer passwords after the function has tried every combination possible.
 The program will split up the first bytes from the alphabet into the available threads.

 use -n <N> to override the start position of the iterator. everything above 5 with current alphabet will take a long time.
 The number of tries will be equal to ABC^N, where ABC is the size of the alphabet, and N is the length of the word. Which means if a character set that contains every possible character in a password, normally it will contain 96 characters, will produce 8 154 726 976 different combinations at 5 length. And with my optimizations and my system with two cores and 4 threads, i can produce ~2 500 000 combinations a minute, which will result in a process time of 56 hours.

# Modifications
 Match ALPHABET: functions.h
