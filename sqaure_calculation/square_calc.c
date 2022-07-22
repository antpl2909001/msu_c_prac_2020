#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define coef 1.0/24	// коэффициент, необходимый для соблюдения точности вычисления фигуры (вычислен аналитически, см. отчет)

double f1(double x){
	// f1(x) = e^(-x) + 3
	double res;

	int a = -1; // коэффициенты для функции f1(x) = e^(-x) + 3
	int b = 3;	// 
	
	__asm__(
		".intel_syntax noprefix\n\t"
		"finit\n\t"
	    "fld %1\n\t"			// ST(0) = x
	    "fild %2\n\t"			// ST(0) = (-1)
	    "fmulp\n\t"         	// ST(0) = (-x)
	    "fldl2e\n\t"        	// ST(0) = log2(e)
	    "fmulp\n\t"         	// ST(0) = (-x)*log2(e)
	    "fld1\n\t"          	// ST(0) = 1  
	    "fld ST(1)\n\t"     	// ST(0) = (-x)*log2(e)
	    "fprem\n\t"         	// ST(0) = (-x*log2(e)) % 1) - дробная часть числа -x*log2(e)
		"fld ST(2)\n\t"			// ST(0) = (-x)*log2(e)
	    "fsub ST(0), ST(1)\n\t"	// ST(0) = (-x)*log2(e) // 1
	    "frndint\n\t"			// ST(0) - целая часть числа -x*log2(e)
	    "fld1\n\t"				// ST(0) = 1 
	    "fscale\n\t"        	// ST(0) = ST(0)*2^ST(1) = 2^(целая часть числа -x*log2(e))
	    "fld ST(2)\n\t"			// 
	    "f2xm1\n\t"				// ST(0) = 2^ST(0) - 1 = 2^(дрбоная часть числа -x*log2(e)) - 1
	    "fld1\n\t"				//
	    "faddp\n\t"				// ST(0) = ST(0) + 1 = 2^(дрбоная часть числа -x*log2(e))
	    "fmulp\n\t"				// ST(0) = 2^(-x*log2(e)) = e^(-x)
	    "fild %3\n\t"			
	    "faddp\n\t"				// ST(0) = e^(-x) + 3
	    "fstp %0\n\t"
	    :"=m"(res)
	    :"m"(x), "m"(a), "m"(b)
	    :
	);

	return res;
}

double f2(double x){
	// f2(x) = 2*x - 2
	double res;

	int a = 2; // коэффициенты для функции f2(x) = 2*x - 2
	int b = -2;	// 
	
	__asm__(
		".intel_syntax noprefix\n\t"
		"finit\n\t"					
		"fld %1\n\t"				// ST(0) = x
		"fild %2\n\t"				// ST(0) = 2
		"fmulp\n\t"					// ST(0) = 2*x
		"fild %3\n\t"				// ST(0) = -2
		"faddp\n\t"					// ST(0) = 2*x - 2
		"fstp %0\n\t"
	    :"=m"(res)
	    :"m"(x), "m"(a), "m"(b)
	    :
	);

	return res;
}

double f3(double x){
	// f3(x) = 1 / x
	double res;

	__asm__(
		".intel_syntax noprefix\n\t"
		"finit\n\t"
		"fld %1\n\t"				// ST(0) = x
		"fld1\n\t"					// ST(0) = 1
		"fdivrp ST(1), ST(0)\n\t"	// ST(0) = 1 / x
		"fstp %0\n\t"
		:"=m"(res)
		:"m"(x)
		:
	);

	return res;
}


int get_monotone(double (*f)(double x), double (*g)(double x), double a, double b){
	// функция возвращает 1, если (f - g) возрастает, и -1, если убывает 
	return f(a) - g(a) > 0 ? -1 : 1;
}

int get_convexity(double (*f)(double x), double (*g)(double x), double a, double b){
	// функция возвращает 1, если (f - g) выпукла вверх, и -1, если выпукла вниз

	double left_val = f(a) - g(a);
	double right_val = f(b) - g(b);
	double mid_val = f((a + b) / 2) - g((a + b) / 2);

	return (mid_val > (left_val + right_val) / 2) ? -1 : 1; 
}

double point(double (*f)(double), double (*g)(double), double a, double b){
	// функция вычисляет точку пересечения хорды функции (f - g)
	// с концами в точках a и b
	double left_val = f(a) - g(a);
	double right_val = f(b) - g(b);
	double c = (a * right_val - b * left_val) / (right_val - left_val);

	return c;
}

double root(double (*f)(double), double (*g)(double), double a, double b, double epsl, int *k){
	// функция root находит корень уравнения f(x)=g(x) 
	// на отрезке [a, b] с точность epsl методом хорд

	if (((f(a) - g(a)) * (f(b) - g(b)) >= 0) || a >= b) {
		printf("Данный отрезок [%f, %f] не удовлетвлоряет условию.\n", a, b);
		printf("Выберете другой отрезок поиска корня.\n");
		exit(1);
	}

	double c = point(f, g, a, b);

	int event = get_monotone(f, g, a, b) * get_convexity(f, g, a, b);
	double x, diff;
	int n = 0;

	if (event > 0){
		do {
			x = point(f, g, c, b);
			diff = x - c;
			c = x;
			n++;
		} while (diff >= epsl);
	} else {
		do {
			x = point(f, g, a, c);
			diff = c - x;
			c = x;
			n++;
		} while (diff >= epsl);
	}

	*k = n; // записали по указателю k кол-во итераций, потребовавшихся для вычисления корня

	return c;
}

double integral(double (*f)(double), double a, double b, double epsl){
	// функция integral находит определенный интеграл функции f(x) 
	// на отрезке [a, b] с точностью epsl методом трапеций

	int n = 20;
	double h = (b - a) / n;

	double func_sum = 0;

	for (int i = 0; i <= n; i++){
		if (i % n == 0){
			func_sum += 0.5 * f(a + i * h);
		} else {
			func_sum += f(a + i * h); 
		}
	}

	double sum_diff = 0; // изменение суммы значений функциии при разбиениях n и 2*n
	double p = 1.0 / 3; // коэффициент p в формуле Рунге

	do {
		n *= 2;
		h /= 2;
		func_sum += sum_diff;
		sum_diff = 0;

		for (int i = 1; i <= n; i += 2){
			sum_diff += f(a + i * h);
		}
	} while (p * h * fabs(func_sum - sum_diff) >= epsl);

	return (func_sum + sum_diff) * h;
}


int main (int argc, char **argv){
	int points_key = 0;		// флаг points_key == 1, если был введен ключ -points
	int iter_key = 0;		// флаг iter_key == 1, если был введен ключ -iter
	// int help_key = 0;		// флаг help_key == 1, если был введен ключ -help
	int root_key = 0;		/* флаг root_key != 0, если был введен ключ -root,
							причем root_key = i, если argv[i] == -root */
	int integral_key = 0;	/* флаг integral_key != 0, если был введен ключ -integral,
							причем integral_key = i, если argv[i] == -integral */

	// Приоритетность обработки ключей:
	// 1) -help - если введен, то обрабатывается только он.
	// 2) -iter или -points - одинаковый приоритет
	// 3) -root или -integral - обрабатывается первый, все остальные элементы argv[] считаются параметрами для функции
	// Параметры, введенные после ключей -root (или -integral) и параметров этой функции игнорируются 

	// ОБРАБОТКА ПАРАМЕТРОВ КОМАНДНОЙ СТРОКИ
	//---------------------------------------------------------------------------------------------
	if (argc >= 2){
		if (!strcmp(argv[1], "-help")){
			printf("-help - вызов справки по ключам данной программы\n");
			printf("-iter - печать кол-ва итераций, потребовавшихся для вычисления точки пересечения функций\n"
				"(игнорируется при ключах -help, -integral)\n");
			printf("-points - вывод абсцисс точек пересечения (игнорируется при ключах -help, -root, -integral)\n");
			printf("-root - тестирование функции root. Далее следуют 5 параметров:\n "
				"n и k - номера функций, a и b - концы отрезка поиска корня, epsl - точность вычисления\n");
			printf("-integral - тестирование функции integral. Далее следуют 4 параметра:\n "
				"n - номер функции, a и b - концы отрезка интегрирвоания, epsl - точность вычисления\n");

			return 0;
		}

		for (int i = 1; i < argc; i++){
			if (!points_key && !strcmp(argv[i], "-points")){
				points_key = 1;
			} else if (!iter_key && !strcmp(argv[i], "-iter")){
				iter_key = 1;
			} else if (!root_key && !strcmp(argv[i], "-root")){
				root_key = i;
				break;
			} else if (!integral_key && !strcmp(argv[i], "-integral")){
				integral_key = i;
				break;
			} else {
				printf("Команда '%s' не найдена\n", argv[i]);

				return 0;
			}
		}
	}
	//---------------------------------------------------------------------------------------------


	int n;	// переменная будет хранить кол-во итераций
	double (*func_ptrs[3])(double) = {f1, f2, f3};	// массив указателей на функции f1, f2, f3

	//---------------------------------------------------------------------------------------------
	// ТЕСТИРОВАНИЕ ФУНКЦИИ ROOT
	if (root_key){
		if (argc - root_key < 5){
			printf("Недостаточно аргументов для функции root\n");
			return 0;
		}

		char *p;
		int i = root_key; // номер элемента root в массиве argv[]

		int f = atoi(argv[i + 1]);
		int g = atoi(argv[i + 2]);

		if (f > 3 || f < 1 || g > 3 || g < 1){
			printf("Неправильный формат параметров функции root\n");
			return 0;
		}

		if (f == g){
			printf("Введите различные функции\n");
			return 0;
		}

		double a = strtod(argv[i + 3], &p);
		if (*p != '\0'){
			printf("Неправильный формат параметров функции root\n");
			return 0;
		}

		double b = strtod(argv[i + 4], &p);
		if (*p != '\0'){
			printf("Неправильный формат параметров функции root\n");
			return 0;
		}

		double epsl = strtod(argv[i + 5], &p);
		if (*p != '\0'){
			printf("Неправильный формат параметров функции root\n");
			return 0;
		}

		double x = root(func_ptrs[f - 1], func_ptrs[g - 1], a, b, epsl, &n);
		printf("Корень уравнения f%d(x) == f%d(x) приблизительно равен %f\n", f, g, x);

		if (iter_key){
			printf("Кол-во итераций: %d\n", n);
		}

		return 0;
	}
	//---------------------------------------------------------------------------------------------




	// ТЕСТИРОВАНИЕ ФУНКЦИИ INTEGRAL
	//---------------------------------------------------------------------------------------------
	if (integral_key){
		if (argc - integral_key < 4){
			printf("Недостаточно аргументов для функции integral\n");
			return 0;
		}

		char *p;
		int i = integral_key; // номер элемента integral в массиве argv[]

		int f = atoi(argv[i + 1]);

		if (f > 3 || f < 1){
			printf("Неправильный формат параметров функции integral\n");
			return 0;
		}

		double a = strtod(argv[i + 2], &p);
		if (*p != '\0'){
			printf("Неправильный формат параметров функции integral\n");
			return 0;
		}

		double b = strtod(argv[i + 3], &p);
		if (*p != '\0'){
			printf("Неправильный формат параметров функции integral\n");
			return 0;
		}

		double epsl = strtod(argv[i + 4], &p);
		if (*p != '\0'){
			printf("Неправильный формат параметров функции integral\n");
			return 0;
		}

		double S = integral(func_ptrs[f - 1], a, b, epsl);
		printf("Интеграл функции f%d(x) на отрезке [%f, %f] с точность %f:\n%f\n", f, a, b, epsl, S);

		return 0;
	}
	//---------------------------------------------------------------------------------------------




	// ОСНОВНАЯ ПРОГРАММА, ВЫЧИСЛЯЮЩАЯ ПЛОЩАДЬ ФИГУРЫ
	//---------------------------------------------------------------------------------------------
	printf("Введите точность вычисления площади: ");

	double epsl;
	scanf("%lf", &epsl);

	double x1 = root(f1, f2, 2, 3, epsl * coef, &n);

	if (points_key){
		printf("Абсцисса пересечения f1(x) и f2(x):\nx1 = %f\n", x1);
	}

	if (iter_key){
		printf("Корень f1(x) = f2(x)	Кол-во итераций: %d\n", n);
	}

	double x2 = root(f1, f3, 0.1, 1, epsl * coef, &n);

	if (points_key){
		printf("Абсцисса пересечения f1(x) и f3(x):\nx2 = %f\n", x2);
	}

	if (iter_key){
		printf("Корень f1(x) = f3(x)	Кол-во итераций: %d\n", n);
	}

	double x3 = root(f2, f3, 1, 2, epsl * coef, &n);

	if (points_key){
		printf("Абсцисса пересечения f2(x) и f3(x):\nx3 = %f\n", x3);
	}

	if (iter_key){
		printf("Корень f2(x) = f3(x)	Кол-во итераций: %d\n", n);
	}
	

	double S1 = integral(f1, x2, x1, epsl * coef);
	double S2 = integral(f2, x3, x1, epsl * coef);
	double S3 = integral(f3, x2, x3, epsl * coef);

	printf("\nПлощадь искомой фигуры, вычисленная с точностью %f, равна: %f\n", epsl, S1 - S2 - S3);

	return 0;
}