int main(int argc, char const *argv[])
{
	char buffer [100];
	execl("test","test",buffer,(char*)buffer);
	return 0;
}