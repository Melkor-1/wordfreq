# wordfreq
 Count Frequency of Words and Print Them in Sorted Order

## Objective:
Write a program to count the frequencies of unique words from standard input, then print them out with their frequencies, ordered most frequent first. For example, given this input:
```console
The foo the foo the
defenestration the
```

The program should print the following:

```console
the 4
foo 2
defenestration 1
```

## Benchmarking:
```bash
» time ./wordfreq < kjvbible_10.txt 1> /dev/null   
./wordfreq < kjvbible_10.txt > /dev/null  2.39s user 0.10s system 99% cpu 2.507 total

-----------------------------------------------------------------------------------
» time wc kjvbible_10.txt
  998170  8211330 43325060 kjvbible_10.txt
wc kjvbible_10.txt  1.12s user 0.02s system 98% cpu 1.156 total
```
