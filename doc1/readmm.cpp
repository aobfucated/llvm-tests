

int main()
{
	void* p = (void*)0x140000000;
	*(char*)p = 0;
	
	return 0;
}