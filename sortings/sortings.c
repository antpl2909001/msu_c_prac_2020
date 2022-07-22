#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INC_LEN 10

long long int random_long(void){
	return rand() * ((long long) rand() << 32); // генерируем long long int конкатенацией двух int-ов
}

void swap(long long *a, long long *b){
	long long tmp = *a;
	*a = *b;
	*b = tmp;
}

int cmp_order(const void *a, const void *b){ // функция сравнения для упорядочивания массива по неубыванию
	long long x = *(long long int *) a;
	long long y = *(long long int *) b;

	if (x > y){
		return 1;
	} else if (x < y){
		return -1;
	} else {
		return 0;
	}
}

int cmp_reverse(const void *a, const void *b){ // функция сравнения для упорядочивания массива по невозрастанию
	long long x = *(long long int *) a;
	long long y = *(long long int *) b;

	if (x > y){
		return -1;
	} else if (x < y){
		return 1;
	} else {
		return 0;
	}
}

long long int *generate_arr(int n, int p){
	// p - параметр: p = -1 - генерация невозрастающего, p = 0 - произвольного , p = 1 - неубывающего массивов
	long long int *arr = (long long int *) malloc(sizeof(long long int) * n);

	for (int i = 0 ; i < n; i++){
		arr[i] = random_long();
	}

	if (p == -1){
		qsort(arr, n, sizeof(long long int), cmp_reverse);
	}
	if (p == 1){
		qsort(arr, n, sizeof(long long int), cmp_order);
	}

	return arr;
}

long long int *duplicate_arr(long long int *a, int n){ // дублируем сгенерированый массив а для сравнения сортировок на одних данных 
	long long int *new_arr = (long long int *) malloc(sizeof(long long int) * n);

	for (int i = 0; i < n; i++){
		new_arr[i] = a[i]; 
	}

	return new_arr;
}

void print_arr(long long *a, int n){
	for (int i = 0; i < n; i++){
		printf("%lld ", a[i]);
	}

	printf("\n");
}

void select_sort(long long *a, int n){
	int exch = 0; // кол-во обменов
	int comp = 0; // кол-во сравнений

	for (int i = 0; i < n - 1; i++){
		int k = i; // индекс минимального элемента (начинаем с текущего)

		for (int j = k + 1; j < n; j++){
			if (a[k] > a[j]){ // нашли новый минимальный элемент
				k = j; // запомнили новый минимальный элемент
			}
			comp++;
		}

		if (k != i){
			swap(&a[i], &a[k]); // поменяли текущий элемент с минимальным
			exch++;
		}
	}

	printf("Количество сравнений: %d\n", comp);
	printf("Количество обменов: %d\n", exch);
}

int *increment(int *len, int n){ // функция генерирует массив приращений через формулы Сэджвика
	int p1, p2, p3; 
	p1 = p2 = p3 = 1;

	int size = INC_LEN;
	int *inc = (int *) malloc(sizeof(int) * size);

	int i = -1;
	do {
		if (i + 1 == size){ // динамически увеличиваем длину массива при необходимости
			size *= 2;
			inc = (int *) realloc(inc, sizeof(int) * size);
		}

		if (++i % 2){ // подсчитываем приращение для четного i
			inc[i] = 8 * p1 - 6 * p2 + 1;
		} else { // подсчитываем приращение для нечетного i
			inc[i] = 9 * p1 - 9 * p3 + 1;
			p2 *= 2;
			p3 *= 2;
		}

		p1 *= 2;
	} while (3 * inc[i] < n);

	inc = (int *) realloc(inc, sizeof(int) * i); // выделяем только необходимую для хранения массива память
	*len = i;

	return inc;
}

void shell_sort(long long *a, int n){
	int exch = 0; // кол-во обменов
	int comp = 0; // кол-во сравнений

	int len; // длина массива с приращениями
	int *inc = increment(&len, n); // массив приращений

	for (int i = len - 1; i >= 0; i--){
		int step = inc[i];

		for (int i = step; i < n; i++){
			int j = i;

			while (j >= step){ // циклом while осуществляем сортировку элементов в подмассивах
				comp++;

				if (a[j - step] > a[j]){
					swap(&a[j - step], &a[j]);
					j -= step;
					exch++;
					continue;
				}

				break;
			}
		}

	}

	free(inc);

	printf("Количество сравнений: %d\n", comp);
	printf("Количество обменов: %d\n", exch);
}

int main(int argc, char **argv){
	srand(time(NULL));

	int n = 10;
	printf("Введите длину массива: ");
	scanf("%d", &n);

	int p;
	printf("Введите параметр p:\n(p = 1 - неубывающий массив,\n p = 0 - произвольный массив,\n p = -1 - невозрастающий массив)\n");
	scanf("%d", &p);

	long long int *arr = generate_arr(n, p);
	long long int *dub_arr = duplicate_arr(arr, n);
	//print_arr(arr, n);

	printf("Сортировка простым выбором:\n");
	select_sort(arr, n);
	//print_arr(arr, n);

	printf("\n");

	printf("Сортировка Шелла:\n");
	shell_sort(dub_arr, n);
	//print_arr(dub_arr, n);

	free(arr);
	free(dub_arr);


	return 0;
}