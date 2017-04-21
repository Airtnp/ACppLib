namespace strange
{
	template<typename T1, typename T2>
	struct strange_pack {};

	struct strange_class {};

	template<typename T1, typename T2>
	strange_pack<T1, T2>
		operator % (T1, T2) {}

	template<typename T1, typename T2>
	void strange_recursive(T1 a, T2 b)
	{
		strange_recursive(b, a % b);
	}
}

int main()
{
	strange::strange_class a, b;
	strange::strange_recursive(a, b);
	return 0;
}