qsort:
	gcc -Wall -Wextra -std=c99 -O2 -pthread my_sort.c -o my_sort

merge:
	gcc -Wall -Wextra -std=c99 -O2 -pthread merge_sort_tweak.c -o merge_sort_tweak

test:
	gcc -Wall -Wextra -std=c99 -O2 -pthread my_sort.c -o my_sort
	gcc -Wall -Wextra -std=c99 -O2 -pthread merge_sort_tweak.c -o merge_sort_tweak

	rm -f myqsort
	time ./my_sort 10 10000000 > myqsort
	rm -f myqsort

	rm -f mymerge
	time ./merge_sort_tweak 10 10000000 > mymerge
	rm -f mymerge
