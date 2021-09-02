#include <unistd.h>
#include <sys/types.h>
#include <limits.h>

int	putnbr_llong(int fd, long long n) {
	if (n == LLONG_MIN) return (write(2, "-9223372036854775807", 20));
	if (n < 0) return (write(fd, "-", 1) + putnbr_llong(-n));
	if (n < 10) return (write(fd, (char []){n + 48}, 1));
	return (putnbr_llong(n / 10LL) + putnbr_llong(n % 10LL));
}

int	putnbr_offt(int fd, off_t n) {
	return (putnbr_llong((long long)n));
}

int	putnbr_unsigned(int fd, unsigned n) {
	if (n < 10) return (write(fd, (char []){n + 48}, 1));
	return (putnbr_unsigned(n / 10) + putnbr_unsigned(n % 10));
}
