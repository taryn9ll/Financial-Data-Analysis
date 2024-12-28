#pragma once

#include <vector>
#include <iostream>
#include <iomanip>
using namespace std;
namespace fre
{
	typedef vector<double> Vector;
	typedef vector<Vector> Matrix;
	// overload operators as independent functions
	Vector operator*(const Matrix &C, const Vector &V);
	Vector operator*(const double &a, const Vector &V);
	Vector operator*(const Vector &V, const Vector &W);
	Vector operator+(const double &a, const Vector &V);
	Vector operator-(const Vector &V, const Vector &W);
	Vector operator+(const Vector &V, const Vector &W); // referece to const for V and W, V + W -> U, V and W do not change
	Vector exp(const Vector &V);
	double operator^(const Vector &V, const Vector &W); // scalar operator

	// overload cout for vector, cout every element in the vector
	template <typename T>
	ostream &operator<<(ostream &out, const vector<T> &V)
	{
		int column_width = 12; // 每列宽度
    	int precision = 3;    // 小数点后保留位数
		for (size_t i = 0; i < V.size(); ++i)
		{
			out << setw(column_width) << fixed << setprecision(precision) << V[i];
			if ((i + 1) % 10 == 0) // 每行 10 个元素后换行
			{
				out << endl;
			}
		}

		if (V.size() % 10 != 0) // 如果不是刚好换行，手动加一个换行符
		{
			out << std::endl;
		}

		return out;
	}

	ostream &operator<<(ostream &out, Matrix &W); // Overload cout for Matrix
}
