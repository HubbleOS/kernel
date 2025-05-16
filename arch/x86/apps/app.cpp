// extern "C" long (*syscall)(long num, long a1, long a2, long a3, long a4, long a5, long a6);

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
