qsort:
	gcc -Wall -Wextra -std=c99 -O2 -pthread my_sort.c -o my_sort

merge:
	gcc -Wall -Wextra -std=c99 -O2 -pthread merge_sort_tweak.c -o merge_sort_tweak

test:
	gcc -Wall -Wextra -std=c99 -O2 -pthread my_sort.c -o my_sort
	gcc -Wall -Wextra -std=c99 -O2 -pthread merge_sort_tweak.c -o merge_sort_tweak

	rm -f myqsort
	rm -f mymerge
	time ./my_sort 10 11 > myqsort
	time ./merge_sort_tweak 10 11 > mymerge

	diff -u myqsort mymerge

	rm -f myqsort
	rm -f mymerge
