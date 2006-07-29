#include <iostream>
#include <fstream>
#include <cstring>
#include <msg-native/MSG_native.h>

MSG_native::MSG_native( )
{
  headerpos = datapos = trailerpos = 0;
  for (int i = 0; i < SEVIRI_CHANNELS; i ++) 
  {
    selected_channel[i] = false;
    numberlines[i] = 0;
    startline[i] = 0;
    endline[i] = 0;
    numbercolumns[i] = 0;
    startcolumn[i] = 0;
    endcolumn[i] = 0;
  }
  nchannels = 0; 
}

bool MSG_native::open( char *name )
{
  in.open(name, (std::ios_base::binary | std::ios_base::in));
  if (in.fail())
  {
    std::cerr << "Cannot open input Native file " << name << std::endl;
    return false;
  }
  return true;
}

void MSG_native::close( )
{
  for (int i = 0; i < SEVIRI_CHANNELS; i ++) 
  {
    line[i].clear( );
    selected_channel[i] = false;
    numberlines[i] = 0;
    startline[i] = 0;
    endline[i] = 0;
    numbercolumns[i] = 0;
    startcolumn[i] = 0;
    endcolumn[i] = 0;
  }
  nchannels = 0; 
  if (in) in.close( );
  headerpos = datapos = trailerpos = 0;
  return;
}

MSG_native::~MSG_native( )
{
  close( );
}

void MSG_native::read( )
{
  header.read(in);
  sscanf(header.mph_sph_header.mphinfo[8].c_str( ),
          "%*s : %*d %ld\n", &headerpos);
  sscanf(header.mph_sph_header.mphinfo[9].c_str( ),
          "%*s : %*d %ld\n", &datapos);
  sscanf(header.mph_sph_header.mphinfo[10].c_str( ),
          "%*s : %*d %ld\n", &trailerpos);
  in.seekg(trailerpos, std::ios::beg);
  trailer.read(in);
  in.seekg(datapos, std::ios::beg);

  char bandsel[16];
  sscanf(header.mph_sph_header.mphinfo[39].c_str( ),
         "%*s : %s", bandsel);
  for (int i = 0; i < SEVIRI_CHANNELS; i ++)
  {
    if (bandsel[i] == 'X')
    {
      selected_channel[i] = true;
      nchannels ++;
    }
  }
  long nlines, ncols, startl, stopl, startc, stopc = 0;
  sscanf(header.mph_sph_header.mphinfo[40].c_str( ),
         "%*s : %ld", &startl);
  sscanf(header.mph_sph_header.mphinfo[41].c_str( ),
         "%*s : %ld", &stopl);
  sscanf(header.mph_sph_header.mphinfo[44].c_str( ),
         "%*s : %ld", &nlines);
  sscanf(header.mph_sph_header.mphinfo[42].c_str( ),
         "%*s : %ld", &startc);
  sscanf(header.mph_sph_header.mphinfo[43].c_str( ),
         "%*s : %ld", &stopc);
  sscanf(header.mph_sph_header.mphinfo[45].c_str( ),
         "%*s : %ld", &ncols);

  MSG_native_line aline;
  for (int il = 0; il < nlines; il ++)
  {
    for (int ic = 0; ic < SEVIRI_CHANNELS; ic ++)
    {
      if (! selected_channel[ic]) continue;
      aline.read(in);
      line[ic].push_back(aline);
      if (ic == 11)
      {
        aline.read(in);
        line[ic].push_back(aline);
        aline.read(in);
        line[ic].push_back(aline);
      }
    }
  }
  for (int ic = 0; ic < HRV_CHANNEL; ic ++)
  {
    if (! selected_channel[ic]) continue;
    numberlines[ic] = nlines;
    startline[ic] = startl;
    endline[ic] = stopl;
    numbercolumns[ic] = ncols;
    startcolumn[ic] = startc;
    endcolumn[ic] = stopc;
  }
  if (selected_channel[HRV_CHANNEL])
  {
    sscanf(header.mph_sph_header.mphinfo[46].c_str( ),
           "%*s : %ld", &nlines);
    sscanf(header.mph_sph_header.mphinfo[47].c_str( ),
           "%*s : %ld", &ncols);
    numberlines[HRV_CHANNEL] = nlines;
    startline[HRV_CHANNEL] = startl;
    endline[HRV_CHANNEL] = stopl;
    numbercolumns[HRV_CHANNEL] = ncols;
    startcolumn[HRV_CHANNEL] = startc;
    endcolumn[HRV_CHANNEL] = stopc;
  }
  return;
}

std::ostream& operator<< ( std::ostream& os, MSG_native &m )
{
  os << m.header;
  os << m.trailer;
  for (int ic = 0; ic < m.SEVIRI_CHANNELS; ic ++)
  {
    if (! m.selected_channel[ic]) continue;
    std::list <MSG_native_line>::iterator p = m.line[ic].begin( );
    while (p != m.line[ic].end( ))
    {
      std::cout << *p;
      p ++;
    }
  }
  return os;
}

bool MSG_native::pgmdump(int ic, char *filename)
{
  if (ic < 0 || ic >= SEVIRI_CHANNELS) return 0;
  if (! selected_channel[ic])
    return false;
  std::ofstream os(filename);
  if (!os.good()) return false;

  os << "P5" << std::endl;
  for (int i = 0; i < (int) header.mph_sph_header.mph_lines; i ++)
    os << "# " << header.mph_sph_header.mphinfo[i];
  os << numbercolumns[ic] << " " << numberlines[ic] << std::endl;
  os << "1024" << std::endl;

  unsigned short *s = 0;
  long ns;

  std::list <MSG_native_line>::iterator p = line[ic].begin( );
  while (p != line[ic].end( ))
  {
    p->data.to_sample(&s, &ns);
    os.write((char *) s, ns*sizeof(short));
    p ++;
  }
  os.close( );

  delete [ ] s;

  return true;
}

int MSG_native::lines(int channel)
{
  if (channel < 0 || channel >= SEVIRI_CHANNELS) return 0;
  return numberlines[channel];
}

int MSG_native::pixels(int channel)
{
  if (channel < 0 || channel >= SEVIRI_CHANNELS) return 0;
  return numbercolumns[channel];
}

unsigned short *MSG_native::data(int channel)
{
  if (channel < 0 || channel >= SEVIRI_CHANNELS) return 0;
  if (!selected_channel[channel]) return 0;

  long size = numberlines[channel]*numbercolumns[channel];
  long px = numbercolumns[channel];

  unsigned short *s = new unsigned short[size];
  unsigned short *sp;
  long ns;

  std::list <MSG_native_line>::iterator p = line[channel].begin( );
  int count = 0;
  while (p != line[channel].end( ))
  {
    sp = s+count*px;
    p->data.to_sample(&sp, &ns);
    p ++;
    count ++;
  }
  return s;
}

#ifdef TESTME

#include <Magick++.h>

int main(int argc, char **argv)
{
  MSG_native native;

  if (argc < 2) return 1;
  if (! native.open(argv[1])) return 1;

  native.read( );

  std::cout << native.header;

  // native.pgmdump(1, "image.pgm");

  unsigned short *pixels = native.data(native.IR_10_8_CHANNEL);

  Magick::Image *image = new Magick::Image(
                         native.pixels(native.IR_10_8_CHANNEL),
                         native.lines(native.IR_10_8_CHANNEL),
                         "I", Magick::ShortPixel, pixels);

  image->normalize( );
  image->rotate(180.0);
  image->write("image.jpg");

  native.close( );
  return 0;
}

#endif

