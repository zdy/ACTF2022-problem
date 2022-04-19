#include <iostream>
#include <Windows.h>

using namespace std;

#define MAXN 9999
#define MAXSIZE 10
#define DLEN 4

class BigNum
{
private:
	int a[500];    //可以控制大数的位数 
	int len;       //大数长度
public:
	BigNum() { len = 1; memset(a, 0, sizeof(a)); }   //构造函数
	BigNum(const int);       //将一个int类型的变量转化为大数
	BigNum(const char*);     //将一个字符串类型的变量转化为大数
	BigNum(const BigNum&);  //拷贝构造函数
	BigNum& operator=(const BigNum&);   //重载赋值运算符，大数之间进行赋值运算

	friend istream& operator>>(istream&, BigNum&);   //重载输入运算符
	friend ostream& operator<<(ostream&, BigNum&);   //重载输出运算符

	BigNum operator+(const BigNum&) const;   //重载加法运算符，两个大数之间的相加运算 
	BigNum operator-(const BigNum&) const;   //重载减法运算符，两个大数之间的相减运算 
	BigNum operator*(const BigNum&) const;   //重载乘法运算符，两个大数之间的相乘运算 
	BigNum operator/(const int&) const;    //重载除法运算符，大数对一个整数进行相除运算

	BigNum operator^(const int&) const;    //大数的n次方运算
	int    operator%(const int&) const;    //大数对一个int类型的变量进行取模运算    
	bool   operator>(const BigNum& T)const;   //大数和另一个大数的大小比较
	bool   operator>(const int& t)const;      //大数和一个int类型的变量的大小比较

	void print();       //输出大数
	bool equal_zero();
};

bool BigNum::equal_zero() {
	if (this->len == 1 && this->a[0] == 0) {
		return true;
	}
	return false;
}

BigNum::BigNum(const int b)     //将一个int类型的变量转化为大数
{
	int c, d = b;
	len = 0;
	memset(a, 0, sizeof(a));
	while (d > MAXN)
	{
		c = d - (d / (MAXN + 1)) * (MAXN + 1);
		d = d / (MAXN + 1);
		a[len++] = c;
	}
	a[len++] = d;
}
BigNum::BigNum(const char* s)     //将一个字符串类型的变量转化为大数
{
	int t, k, index, l, i;
	memset(a, 0, sizeof(a));
	l = strlen(s);
	len = l / DLEN;
	if (l % DLEN)
		len++;
	index = 0;
	for (i = l - 1; i >= 0; i -= DLEN)
	{
		t = 0;
		k = i - DLEN + 1;
		if (k < 0)
			k = 0;
		for (int j = k; j <= i; j++)
			t = t * 10 + s[j] - '0';
		a[index++] = t;
	}
}
BigNum::BigNum(const BigNum& T) : len(T.len)  //拷贝构造函数
{
	int i;
	memset(a, 0, sizeof(a));
	for (i = 0; i < len; i++)
		a[i] = T.a[i];
}
BigNum& BigNum::operator=(const BigNum& n)   //重载赋值运算符，大数之间进行赋值运算
{
	int i;
	len = n.len;
	memset(a, 0, sizeof(a));
	for (i = 0; i < len; i++)
		a[i] = n.a[i];
	return *this;
}
istream& operator>>(istream& in, BigNum& b)   //重载输入运算符
{
	char ch[MAXSIZE * 4];
	int i = -1;
	in >> ch;
	int l = strlen(ch);
	int count = 0, sum = 0;
	for (i = l - 1; i >= 0;)
	{
		sum = 0;
		int t = 1;
		for (int j = 0; j < 4 && i >= 0; j++, i--, t *= 10)
		{
			sum += (ch[i] - '0') * t;
		}
		b.a[count] = sum;
		count++;
	}
	b.len = count++;
	return in;

}
ostream& operator<<(ostream& out, BigNum& b)   //重载输出运算符
{
	int i;
	cout << b.a[b.len - 1];
	for (i = b.len - 2; i >= 0; i--)
	{
		cout.width(DLEN);
		cout.fill('0');
		cout << b.a[i];
	}
	return out;
}

BigNum BigNum::operator+(const BigNum& T) const   //两个大数之间的相加运算
{
	BigNum t(*this);
	int i, big;      //位数   
	big = T.len > len ? T.len : len;
	for (i = 0; i < big; i++)
	{
		t.a[i] += T.a[i];
		if (t.a[i] > MAXN)
		{
			t.a[i + 1]++;
			t.a[i] -= MAXN + 1;
		}
	}
	if (t.a[big] != 0)
		t.len = big + 1;
	else
		t.len = big;
	return t;
}
BigNum BigNum::operator-(const BigNum& T) const   //两个大数之间的相减运算 
{
	int i, j, big;
	bool flag;
	BigNum t1, t2;
	if (*this > T)
	{
		t1 = *this;
		t2 = T;
		flag = 0;
	}
	else
	{
		t1 = T;
		t2 = *this;
		flag = 1;
	}
	big = t1.len;
	for (i = 0; i < big; i++)
	{
		if (t1.a[i] < t2.a[i])
		{
			j = i + 1;
			while (t1.a[j] == 0)
				j++;
			t1.a[j--]--;
			while (j > i)
				t1.a[j--] += MAXN;
			t1.a[i] += MAXN + 1 - t2.a[i];
		}
		else
			t1.a[i] -= t2.a[i];
	}
	t1.len = big;
	while (t1.a[t1.len - 1] == 0 && t1.len > 1)
	{
		t1.len--;
		big--;
	}
	if (flag)
		t1.a[big - 1] = 0 - t1.a[big - 1];
	return t1;
}

BigNum BigNum::operator*(const BigNum& T) const   //两个大数之间的相乘运算 
{
	BigNum ret;
	int i, j, up;
	int temp, temp1;
	for (i = 0; i < len; i++)
	{
		up = 0;
		for (j = 0; j < T.len; j++)
		{
			temp = a[i] * T.a[j] + ret.a[i + j] + up;
			if (temp > MAXN)
			{
				temp1 = temp - temp / (MAXN + 1) * (MAXN + 1);
				up = temp / (MAXN + 1);
				ret.a[i + j] = temp1;
			}
			else
			{
				up = 0;
				ret.a[i + j] = temp;
			}
		}
		if (up != 0)
			ret.a[i + j] = up;
	}
	ret.len = i + j;
	while (ret.a[ret.len - 1] == 0 && ret.len > 1)
		ret.len--;
	return ret;
}
BigNum BigNum::operator/(const int& b) const   //大数对一个整数进行相除运算
{
	BigNum ret;
	int i, down = 0;
	for (i = len - 1; i >= 0; i--)
	{
		ret.a[i] = (a[i] + down * (MAXN + 1)) / b;
		down = a[i] + down * (MAXN + 1) - ret.a[i] * b;
	}
	ret.len = len;
	while (ret.a[ret.len - 1] == 0 && ret.len > 1)
		ret.len--;
	return ret;
}
int BigNum::operator %(const int& b) const    //大数对一个int类型的变量进行取模运算    
{
	int i, d = 0;
	for (i = len - 1; i >= 0; i--)
	{
		d = ((d * (MAXN + 1)) % b + a[i]) % b;
	}
	return d;
}
BigNum BigNum::operator^(const int& n) const    //大数的n次方运算
{
	BigNum t, ret(1);
	int i;
	if (n < 0)
		exit(-1);
	if (n == 0)
		return 1;
	if (n == 1)
		return *this;
	int m = n;
	while (m > 1)
	{
		t = *this;
		for (i = 1; i << 1 <= m; i <<= 1)
		{
			t = t * t;
		}
		m -= i;
		ret = ret * t;
		if (m == 1)
			ret = ret * (*this);
	}
	return ret;
}
bool BigNum::operator>(const BigNum& T) const   //大数和另一个大数的大小比较
{
	int ln;
	if (len > T.len)
		return true;
	else if (len == T.len)
	{
		ln = len - 1;
		while (a[ln] == T.a[ln] && ln >= 0)
			ln--;
		if (ln >= 0 && a[ln] > T.a[ln])
			return true;
		else
			return false;
	}
	else
		return false;
}
bool BigNum::operator >(const int& t) const    //大数和一个int类型的变量的大小比较
{
	BigNum b(t);
	return *this > b;
}

void BigNum::print()    //输出大数
{
	int i;
	cout << a[len - 1];
	for (i = len - 2; i >= 0; i--)
	{
		//cout.width(DLEN);
		//cout.fill('0');
		cout << a[i] << " ";
	}
	cout << endl;
}



class Myflag
{
public:
    Myflag(BigNum flag);
    ~Myflag();
    virtual  void __cdecl Auth(BigNum* flag);
    BigNum result;
};
 
Myflag::Myflag(BigNum num)
{
	result = num;
}

Myflag::~Myflag()
{

}

string decode(int* encoded, int size)
{
	// i put it in a for so one can modify the encoding to something more complex
	string s = "";
	for (int i = 0; i < size; i++)
	{
		s += char(encoded[i] ^ 0xff);
	}
	return s;
}

int fakenum1[] = { 204, 204, 207, 204, 204, 202, 204, 204, 207, 204, 204, 202, 204, 204, 207, 204, 204, 202, 204, 204, 207, 204, 204, 202, 204, 204, 207, 204, 204, 202, 204, 204, 207, 204, 204, 202, 204, 204, 207, 204, 204, 202, 204, 204, 207, 204, 204, 202 };
int fakenum2[] = { 207, 202, 201, 203, 204, 204, 202, 207, 202, 201, 203, 204, 204, 202, 207, 202, 201, 203, 204, 204, 202, 207, 202, 201, 203, 204, 204, 202, 207, 202, 201, 203, 204, 204, 202, 207, 202, 201, 203, 204, 204, 202 };
int fakenum3[] = { 206, 204, 198, 205, 207, 206, 207, 206, 204, 198, 205, 207, 206, 207, 206, 204, 198, 205, 207, 206, 207, 206, 204, 198, 205, 207, 206, 207, 206, 204, 198, 205, 207, 206, 207, 206, 204, 198, 205, 207, 206, 207 };
int fakenum4[] = { 205, 203, 201, 203, 204, 200, 205, 203, 201, 203, 204, 200, 205, 203, 201, 203, 204, 200, 205, 203, 201, 203, 204, 200, 205, 203, 201, 203, 204, 200, 205, 203, 201, 203, 204, 200 };
int fakenum5[] = { 204, 205, 207, 200, 200, 200, 199, 204, 205, 207, 200, 200, 200, 199, 204, 205, 207, 200, 200, 200, 199, 204, 205, 207, 200, 200, 200, 199, 204, 205, 207, 200, 200, 200, 199, 204, 205, 207, 200, 200, 200, 199 };
int fakenum6[] = { 204, 202, 198, 205, 202, 199, 204, 202, 198, 205, 202, 199, 204, 202, 198, 205, 202, 199, 204, 202, 198, 205, 202, 199, 204, 202, 198, 205, 202, 199, 204, 202, 198, 205, 202, 199 };
int fakenum7[] = { 203, 203, 205, 205, 204, 202, 203, 203, 205, 205, 204, 202, 203, 203, 205, 205, 204, 202, 203, 203, 205, 205, 204, 202, 203, 203, 205, 205, 204, 202, 203, 203, 205, 205, 204, 202 };
int fakenum8[] = { 203, 203, 207, 205, 198, 202, 203, 203, 207, 205, 198, 202, 203, 203, 207, 205, 198, 202, 203, 203, 207, 205, 198, 202, 203, 203, 207, 205, 198, 202, 203, 203, 207, 205, 198, 202 };
int fakenum9[] = { 202, 205, 207, 206, 204, 206, 203, 202, 205, 207, 206, 204, 206, 203, 202, 205, 207, 206, 204, 206, 203, 202, 205, 207, 206, 204, 206, 203, 202, 205, 207, 206, 204, 206, 203, 202, 205, 207, 206, 204, 206, 203 };
int fakenum10[] = { 202, 203, 202, 205, 199, 204, 207, 202, 203, 202, 205, 199, 204, 207, 202, 203, 202, 205, 199, 204, 207, 202, 203, 202, 205, 199, 204, 207, 202, 203, 202, 205, 199, 204, 207, 202, 203, 202, 205, 199, 204, 207 };

int real_num1[] = { 201, 203, 202, 199, 203, 202, 203, 207, 205, 198, 206, 199, 200, 205, 202, 206, 201, 201, 205, 200, 199, 198, 203, 198, 204, 198, 202, 198, 207, 201, 199, 203, 198, 202, 206, 200, 207, 204, 203, 200, 198, 201, 203, 204, 204, 200, 206, 204, 199, 206, 203, 205, 207, 203, 204, 203, 201, 198, 199, 201, 200, 201, 206, 198, 205, 198, 206, 201, 206, 205, 201, 199, 207, 205, 200, 199, 198, 204, 199, 199 };
int real_num2[] = { 206, 206, 200, 199, 204, 203, 206, 207, 203, 206, 207, 203, 201, 198, 200, 204, 199, 207, 203, 199, 205, 199, 204, 206, 202, 205, 206, 200, 206, 199, 198, 199, 202, 207, 200, 201, 200, 198, 202, 204, 200, 199, 206, 205, 201, 204, 203, 199, 203, 206, 207, 204, 205, 207, 202, 202, 204, 201, 206, 201, 205, 205, 198, 199, 198, 202, 200, 202, 202, 201, 205, 206, 205, 206, 204, 205, 204, 202, 205, 201 };
int real_num3[] = { 202, 202, 203, 203, 207, 199, 202, 206, 200, 200, 200, 201, 200, 198, 206, 199, 203, 203, 206, 199, 198, 200, 205, 202, 199, 206, 207, 198, 206, 200, 198, 201, 202, 199, 205, 204, 205, 206, 207, 207, 206, 202, 206, 200, 200, 204, 205, 199, 201, 199, 202, 207, 198, 198, 203, 200, 200, 206, 201, 203, 202, 204, 203, 206, 203, 206, 207, 198, 207, 205, 202, 207, 204, 201, 202, 207, 201, 200, 198, 204 };
int real_num4[] = { 206, 200, 199, 201, 200, 207, 203, 200, 202, 199, 198, 206, 200, 206, 203, 200, 200, 202, 200, 203, 199, 203, 200, 200, 204, 200, 198, 206, 205, 204, 205, 199, 200, 202, 204, 206, 207, 199, 199, 203, 198, 204, 207, 203, 202, 203, 198, 205, 199, 207, 205, 207, 202, 198, 198, 205, 205, 207, 203, 202, 199, 200, 200, 201, 207, 204, 201, 206, 204, 206, 207, 204, 206, 200, 198, 199, 204, 201, 207, 200 };
int real_num5[] = { 207, 200, 202, 204, 200, 204, 207, 205, 200, 207, 201, 202, 199, 205, 204, 198, 206, 205, 204, 199, 199, 202, 204, 199, 206, 200, 203, 199, 204, 201, 207, 207, 205, 205, 199, 200, 204, 204, 203, 200, 198, 204, 204, 204, 206, 202, 205, 203, 199, 199, 205, 206, 199, 203, 200, 200, 199, 203, 207, 206, 203, 198, 199, 203, 200, 206, 199, 198, 207, 203, 198, 202, 206, 201, 198, 202, 205, 200, 199, 200 };
int real_num6[] = { 199, 207, 200, 198, 204, 205, 205, 201, 198, 204, 202, 201, 198, 198, 205, 198, 202, 199, 205, 203, 201, 206, 199, 202, 206, 198, 201, 199, 202, 201, 204, 199, 199, 207, 198, 199, 200, 203, 202, 200, 198, 204, 203, 204, 204, 203, 205, 202, 201, 203, 200, 206, 205, 203, 206, 198, 205, 204, 202, 202, 199, 200, 206, 200, 200, 200, 206, 204, 206, 201, 202, 202, 207, 205, 206, 205, 206, 201, 201, 203 };
int real_num7[] = { 206, 203, 204, 199, 202, 205, 199, 204, 205, 205, 201, 201, 199, 198, 206, 200, 206, 202, 205, 204, 203, 203, 202, 199, 203, 203, 204, 199, 199, 200, 201, 198, 203, 201, 200, 205, 204, 205, 207, 205, 204, 203, 206, 206, 203, 201, 200, 204, 198, 203, 203, 205, 205, 198, 199, 207, 203, 207, 204, 200, 205, 198, 199, 203, 199, 201, 204, 206, 201, 206, 198, 204, 207, 199, 202, 200, 198, 202, 198, 198 };
int real_num8[] = { 202, 202, 207, 200, 198, 207, 205, 198, 200, 200, 205, 199, 203, 207, 206, 204, 199, 206, 203, 202, 200, 199, 202, 207, 207, 202, 201, 207, 206, 204, 203, 207, 204, 205, 202, 200, 199, 198, 201, 200, 202, 201, 201, 199, 199, 206, 200, 202, 201, 206, 207, 203, 202, 203, 207, 204, 206, 200, 204, 201, 202, 198, 205, 205, 204, 204, 200, 200, 204, 203, 201, 200, 205, 200, 205, 198, 202, 200, 203, 198 };
int real_num9[] = { 200, 206, 206, 206, 198, 204, 204, 205, 203, 202, 200, 205, 207, 205, 199, 201, 204, 201, 200, 206, 198, 205, 205, 207, 203, 202, 205, 205, 203, 198, 207, 202, 204, 199, 203, 201, 205, 207, 200, 203, 205, 198, 206, 205, 198, 203, 198, 207, 201, 202, 206, 198, 207, 205, 200, 203, 206, 200, 204, 200, 205, 203, 201, 199, 199, 200, 201, 203, 205, 200, 205, 204, 206, 204, 198, 207, 207, 203, 201, 202 };
int real_num10[] = { 202, 200, 200, 207, 202, 202, 200, 204, 198, 202, 205, 203, 203, 198, 201, 198, 198, 201, 205, 207, 207, 200, 205, 206, 207, 203, 207, 202, 202, 207, 204, 207, 207, 205, 202, 199, 199, 201, 198, 199, 203, 206, 199, 207, 202, 207, 207, 200, 204, 203, 204, 199, 205, 205, 202, 207, 202, 199, 200, 206, 202, 205, 203, 206, 200, 207, 203, 207, 206, 203, 206, 201, 200, 198, 202, 198, 199, 199, 198, 203 };

int real_flag[] = { 199, 204, 203, 202, 200, 205, 207, 202, 206, 199, 206, 203, 204, 204, 200, 207, 200, 207, 203, 201, 198, 200, 203, 203, 202, 202, 198, 200, 201, 206, 206, 198, 198, 201, 207, 202, 206, 205, 206, 199, 207, 202, 200, 205, 199, 201, 205, 205, 201, 206, 198, 203, 199, 207, 207, 204, 198, 199, 198, 203, 203, 207, 200, 206, 201, 200, 206, 202, 205, 201, 206, 205, 203, 200, 207, 199, 203, 205, 203, 200, 200, 199, 206, 204, 198, 203, 206, 206, 205, 207, 200, 199, 207, 204, 200, 203, 202, 200, 207, 205, 207, 202, 198, 204, 207, 198, 202, 205, 199, 199, 204, 201, 201, 206, 207, 207, 207, 198, 198, 199, 200, 206, 202, 206, 207, 200, 205, 204, 206, 201, 198, 202, 198, 206, 198, 207, 207, 206, 205, 204, 199, 199, 206, 199, 199, 200, 198, 198, 203, 203, 200, 200, 204, 202, 206, 201, 202, 207, 200, 204, 201, 201, 199, 201, 202, 201, 204, 204, 199, 199, 201, 198, 201, 201, 204, 204, 207, 198, 206, 205, 206, 202, 201, 203, 207, 205, 207, 201, 204, 200, 204, 202, 204, 207, 201, 204, 207, 204, 198, 201, 201, 206, 198, 204, 203, 199, 206, 201, 202, 199, 207, 201, 201, 203, 204, 200, 202, 201, 204, 202, 199, 200, 205, 203, 206, 200, 206, 199, 207, 204, 201, 202, 201, 205, 203, 199, 207, 203, 198, 201, 204, 201, 199, 202, 198, 205, 206, 198, 203, 200, 206, 198, 207, 198, 205, 204, 204, 198, 199, 201, 199, 202, 206, 205, 200, 200, 204, 205, 205, 205, 200, 206, 206, 201, 207, 207, 199, 200, 199, 200, 199, 205, 198, 207, 204, 206, 207, 198, 198, 203, 198, 200, 200, 198, 205, 203, 202, 202, 207, 207, 207, 198, 199, 201, 207, 201, 202, 200, 207, 205, 203, 199, 199, 204, 207, 202, 200, 207, 200, 198, 205, 207, 205, 199, 199, 204, 206, 206, 204, 204, 198, 203, 198, 203, 203, 207, 206, 201, 203, 205, 206, 198, 199, 203, 205, 199, 200, 206, 207, 204, 203, 205, 200, 202, 198, 204, 199, 203, 204, 204 };


void __cdecl Myflag::Auth(BigNum* flag)
{
	BigNum num1 = BigNum(decode(fakenum1, sizeof(fakenum1) / sizeof(fakenum1[0])).c_str());
	BigNum num2 = BigNum(decode(fakenum2, sizeof(fakenum2) / sizeof(fakenum2[0])).c_str());
	BigNum num3 = BigNum(decode(fakenum3, sizeof(fakenum3) / sizeof(fakenum3[0])).c_str());
	BigNum num4 = BigNum(decode(fakenum4, sizeof(fakenum4) / sizeof(fakenum4[0])).c_str());
	BigNum num5 = BigNum(decode(fakenum5, sizeof(fakenum5) / sizeof(fakenum5[0])).c_str());
	BigNum num6 = BigNum(decode(fakenum6, sizeof(fakenum6) / sizeof(fakenum6[0])).c_str());
	BigNum num7 = BigNum(decode(fakenum7, sizeof(fakenum7) / sizeof(fakenum7[0])).c_str());
	BigNum num8 = BigNum(decode(fakenum8, sizeof(fakenum8) / sizeof(fakenum8[0])).c_str());
	BigNum num9 = BigNum(decode(fakenum9, sizeof(fakenum9) / sizeof(fakenum9[0])).c_str());
	BigNum num10 = BigNum(decode(fakenum10, sizeof(fakenum10) / sizeof(fakenum10[0])).c_str());
	BigNum res = *flag;
	res = res + num1;
	res = res * num2;
	res = res - num3;
	res = res + num4;
	res = res * num5;
	res = res - num6;
	res = res + num7;
	res = res - num8;
	res = res + num9;
	res = res - num10;
	*flag = res;
	return ;
}
 
void __cdecl real_auth(void* p, BigNum *flag)
{
	BigNum num1 = BigNum(decode(real_num1, sizeof(real_num1) / sizeof(real_num1[0])).c_str());
	BigNum num2 = BigNum(decode(real_num2, sizeof(real_num2) / sizeof(real_num2[0])).c_str());
	BigNum num3 = BigNum(decode(real_num3, sizeof(real_num3) / sizeof(real_num3[0])).c_str());
	BigNum num4 = BigNum(decode(real_num4, sizeof(real_num4) / sizeof(real_num4[0])).c_str());
	BigNum num5 = BigNum(decode(real_num5, sizeof(real_num5) / sizeof(real_num5[0])).c_str());
	BigNum num6 = BigNum(decode(real_num6, sizeof(real_num6) / sizeof(real_num6[0])).c_str());
	BigNum num7 = BigNum(decode(real_num7, sizeof(real_num7) / sizeof(real_num7[0])).c_str());
	BigNum num8 = BigNum(decode(real_num8, sizeof(real_num8) / sizeof(real_num8[0])).c_str());
	BigNum num9 = BigNum(decode(real_num9, sizeof(real_num9) / sizeof(real_num9[0])).c_str());
	BigNum num10 = BigNum(decode(real_num10, sizeof(real_num10) / sizeof(real_num10[0])).c_str());
	//cout << flag << endl;
	BigNum res = *flag;
	res = res + num1;
	//cout << num1 << endl;
	//cout << res << endl << endl;
	res = res * num2;
	//cout << num2 << endl;
	//cout << res << endl << endl;
	res = res - num3;
	//cout << num3 << endl;
	//cout << res << endl << endl;
	res = res + num4;
	//cout << num4 << endl;
	//cout << res << endl << endl;
	res = res * num5;
	//cout << num5 << endl;
	//cout << res << endl << endl;
	res = res - num6;
	//cout << num6 << endl;
	//cout << res << endl << endl;
	res = res + num7;
	//cout << num7 << endl;
	//cout << res << endl << endl;
	res = res - num8;
	//cout << num8 << endl;
	//cout << res << endl << endl;
	res = res + num9;
	//cout << num9 << endl;
	//cout << res << endl << endl;
	res = res - num10;
	//cout << num10 << endl;
	//cout << res << endl << endl;
	*flag = res;
	return ;
}

//template<class T, class U>
//void VtlHook(T* pT, int Tidx, U* pU, int Uidx)
//{
//    auto pTVtAddr = (size_t*)*(size_t*)pT;
//    auto pUVtAddr = (size_t*)*(size_t*)pU;
//
//    DWORD dwProct = 0;
//    VirtualProtect(pTVtAddr, sizeof(size_t), PAGE_READWRITE, &dwProct);
//    pTVtAddr[Tidx] = pUVtAddr[Uidx];
//    VirtualProtect(pTVtAddr, sizeof(size_t), dwProct, nullptr);
//};

template<class T>
void VtlHookFun(T* pT, int Tidx, size_t funcAdd)
{
    auto pTVtAddr = (size_t*)*(size_t*)pT;
    DWORD dwProct = 0;
    BOOL dret = VirtualProtect(pTVtAddr, sizeof(size_t), PAGE_READWRITE, &dwProct);
    //printf("%d\n", dret);
    pTVtAddr[Tidx] = funcAdd;
    VirtualProtect(pTVtAddr, sizeof(size_t), dwProct, nullptr);
}


void quickSort(int s[], int l, int r)
{
	if (l < r)
	{
		int i = l, j = r, x = s[l];
		while (i < j)
		{
			while (i < j && s[j] >= x) // 从右向左找第一个小于x的数
				j--;
			if (i < j)
				s[i++] = s[j];
			while (i < j && s[i] < x) // 从左向右找第一个大于等于x的数
				i++;
			if (i < j)
				s[j--] = s[i];
		}
		s[i] = x;
		quickSort(s, l, i - 1); // 递归调用
		quickSort(s, i + 1, r);
	}
}

int que_sort[] = {12, 324, 43, 4343, 431, 3232, 1, 12, 2323, 21, 23, 31, 231, 31, 3, 13, 13, 13, 13231, 32323, 23, 23232, 32, 32, 3232, 32, 0};

void make_exception(Myflag *test)
{
    __try {
		quickSort(que_sort, 0, 26);
		test->result = test->result / que_sort[0];
    }
    __except (GetExceptionCode() == EXCEPTION_INT_DIVIDE_BY_ZERO ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        //VtlHook(pb,0,pHook,0);
        VtlHookFun(test, 0, (size_t)real_auth);
    }
	return ;
}


// decode strings
int flagenc[] = { 153, 147, 158, 152, 197 };
int goodenc[] = { 152, 144, 144, 155 };
int wrongenc[] = { 136, 141, 144, 145, 152 };
//

const char base[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
/* Base64 编码 */
char* base64_encode(const char* data, int data_len)
{
	//int data_len = strlen(data);   
	int prepare = 0;
	int ret_len;
	int temp = 0;
	char* ret = NULL;
	char* f = NULL;
	int tmp = 0;
	char changed[4];
	int i = 0;
	ret_len = data_len / 3;
	temp = data_len % 3;
	if (temp > 0)
	{
		ret_len += 1;
	}
	ret_len = ret_len * 4 + 1;
	ret = (char*)malloc(ret_len);

	if (ret == NULL)
	{
		printf("No enough memory.\n");
		return nullptr;
	}
	memset(ret, 0, ret_len);
	f = ret;
	while (tmp < data_len)
	{
		temp = 0;
		prepare = 0;
		memset(changed, '\0', 4);
		while (temp < 3)
		{
			//printf("tmp = %d\n", tmp);   
			if (tmp >= data_len)
			{
				break;
			}
			prepare = ((prepare << 8) | (data[tmp] & 0xFF));
			tmp++;
			temp++;
		}
		prepare = (prepare << ((3 - temp) * 8));
		//printf("before for : temp = %d, prepare = %d\n", temp, prepare);   
		for (i = 0; i < 4; i++)
		{
			if (temp < i)
			{
				changed[i] = 0x40;
			}
			else
			{
				changed[i] = (prepare >> ((3 - i) * 6)) & 0x3F;
			}
			*f = base[changed[i]];
			//printf("%.2X", changed[i]);   
			f++;
		}
	}
	*f = '\0';

	return ret;

}

BigNum enc1(string s) {
	BigNum res = BigNum();
	for (int i = 0; i < s.size(); ++i) {
		BigNum factor = BigNum(128);
		BigNum tmp = factor ^ i;
		BigNum x = BigNum(int(s[i]));
		BigNum y = x * tmp;
		res = res + y;
	}
	return res;
}

int main()
{
	cout << decode(flagenc, 5);

	string s;
	cin >> s;

	//对flag进行加密
	s = base64_encode(s.c_str(), s.size());
	//cout << s << endl;

	BigNum flag = enc1(s);

	Myflag* test = new Myflag(decode(real_flag, sizeof(real_flag) / sizeof(real_flag[0])).c_str());

	make_exception(test);

	BigNum* ptr = new BigNum(flag);

	////cout << flag << endl;

	test->Auth(ptr);

	BigNum res = *ptr - test->result;
	if (res.equal_zero()) {
		cout << decode(goodenc, 4) << endl;
	}
	else {
		cout << decode(wrongenc, 5) << endl;
	}
	
	delete test;


    /*Myflag* test = new Myflag;

    test->Auth();

    exception1(test);

    test->Auth();

    delete test;

	BigNum flag = BigNum("22222222222222222222222222222222222222222222222222222222222222222222222222222222");
	BigNum num = BigNum("11111111111111111111111111111111111111111111111111111111111111111111111111111111");
	BigNum res = flag * num;
	cout << res << endl;*/

    return 0;
}