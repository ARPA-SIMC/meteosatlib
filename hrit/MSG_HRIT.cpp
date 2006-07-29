#include <hrit/MSG_HRIT.h>

bool MSG_HRIT::open(char *name)
{
  hrit_ifstream.open(name, (std::ios_base::binary | std::ios_base::in));
  if (hrit_ifstream.fail( ))
  {
    std::cerr << "Cannot open input hrit file " << name << std::endl;
    return false;
  }
  return true;
}

void MSG_HRIT::read( )
{
  l15_head.read_from(hrit_ifstream);
  l15_data.read_from(hrit_ifstream, l15_head);
  return;
}

void MSG_HRIT::close( )
{
  hrit_ifstream.close( );
}

std::ostream& operator<< ( std::ostream& os, const MSG_HRIT &hrit )
{
  os << (MSG_header&) hrit.l15_head << (MSG_data&) hrit.l15_data;
  return os;
}

#ifdef TESTME

int main(int argc, char **argv)
{
  MSG_HRIT hrit;

  if (! hrit.open(argv[1]))
    return 1;

  hrit.read( );

  std::cout << hrit;

  hrit.close( );
  return 0;
}

#endif
