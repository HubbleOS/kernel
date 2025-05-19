class Hell
{
private:
public:
	int num;

	Hell(int n) : num(n) {}
};

extern "C" int main()
{
	Hell h(666);
	return h.num;
}
