#include <PdfGen.h>
#include <SaneDev.hpp>

#include <iostream>
#include <unistd.h>
#include <map>
#include <cstring>

using namespace sanepp;
using namespace pdfgen;

enum class Operation {
  DumpOptions,
  Scan
};

void OutputOptions( const SaneDev& dev, std::vector<sanepp::Option> options)
{
  Status status;
  std::cout<<"Options: "<<options.size()<<std::endl;
  for (int i=0; i<options.size(); ++i )
  {
    auto option = options[i];
    std::cout<<" "<<option.name<<std::endl<<"  Name: "<<option.title<<std::endl<<"  Size: "<<option.size<<(option.readonly?" (Readonly) ":" ")<<std::endl;
    switch( option.type )
    {
    case sanepp::OptionValueType::Boolean:
      {
        bool boolval = 0;
        status = dev.GetOption( option.name, boolval);
        std::cout<<"    Bool Value: "<<boolval<<std::endl;
      }
      break;
    case sanepp::OptionValueType::Integer:
      {
        int intval = 0;
        status = dev.GetOption( option.name, intval);
        std::cout<<"    Int Value: "<<intval<<std::endl;
	if (!option.constraintIntegerList.empty())
	  std::cout<<"    Constraints: "<<std::endl;
        for ( auto intVal : option.constraintIntegerList )
        {
            std::cout<<"         "<<intVal<<std::endl;
        }
        if ( option.constraintIntegerRange.min != option.constraintIntegerRange.max)
        {
            std::cout<<"    Constraint range: "<<(option.constraintIntegerRange.min)<<" - "<<(option.constraintIntegerRange.max)<<" Step: "<<(option.constraintIntegerRange.quant)<<std::endl;
        }
      }
      break;
    case sanepp::OptionValueType::String:
      {
        std::string strval;
        status = dev.GetOption( option.name, strval);
        std::cout<<"    Str Value: "<<strval<<std::endl;
	if (!option.constraintStringList.empty())
          std::cout<<"    Constraints: "<<std::endl;
        for ( auto strVal : option.constraintStringList )
        {
          std::cout<<"         "<<strVal<<std::endl;
        }
      }
      break;
    case sanepp::OptionValueType::Float:
      {
        double dval = 0.0;
        status = dev.GetOption( option.name, dval);
        std::cout<<"    Float Value: "<<dval<<std::endl;
        if ( option.constraintFloatRange.min != option.constraintFloatRange.max)
        {
          std::cout<<"    Constraint range: "<<(option.constraintFloatRange.min)<<" - "<<(option.constraintFloatRange.max)<<" Step: "<<(option.constraintFloatRange.quant)<<std::endl;
        }
      }
      break;
    }
  }
}

int main( int argc, char* argv[] )
{
  int deviceIdx = 0;
  int verbose = 0;
  const char* outputfile = "output.pdf";
  SaneDev dev;
  PdfGen pdf;
  Status status;
  std::vector<SaneDevDescriptor> deviceList;
  Operation op = Operation::Scan;
  bool annotate = false;

  std::map<std::string,std::string> selectedOptions;
  int c;
  while ( (c = getopt(argc, argv, "ahvLOd:f:o:") ) != -1 )
  {
    switch(c)
    {
    case 'h':
      std::cout<<"Usage: "<<argv[0]<<" [-h] [-v] [-L] [-f filename] [-d devicenumber] [-O] [-o optionname=optionvalue]"<<std::endl<<"  -O\tList options"<<std::endl<<"  -L\tList devices"<<std::endl;  
      return 0;
    case 'v':
      verbose++;
      break;
    case 'L':
      std::cout<<"Devices:"<<std::endl;
      status = dev.GetDevices( deviceList );
      if (status != Status::Good )
      {
        std::cerr<<"Failed to get device list"<<std::endl;
        return -1;
      }
      for (auto& device : deviceList)
      {
        std::cout<<"\t"<<device.name<<std::endl;
      }
      return 0;
    case 'd':
      sscanf(optarg, "%d", &deviceIdx);
      break;
    case 'f':
      outputfile = optarg;
      break;
    case 'O':
      op = Operation::DumpOptions;
      break;
    case 'a':
      annotate = true;
      break;
    case 'o':
      char* optVal = strchr(optarg,'=');
      if (optVal)
      {
        optVal[0] = '\0';
        optVal++;
        selectedOptions.insert( std::make_pair<std::string,std::string>(optarg,optVal));
      }
      break;
    }
  }
  status = dev.GetDevices( deviceList );
  if (status != Status::Good )
  {
    std::cerr<<"Failed to get device list"<<std::endl;
    return -1;
  }
  if (deviceIdx < 0)
  {
    std::cerr<<"Invalid device index: "<<deviceIdx<<std::endl;
    return -1;
  }
  if (deviceIdx >= deviceList.size() )
  {
    std::cerr<<"Device not found at index "<<deviceIdx<<std::endl;
    return -1;
  }

  status = dev.Open( deviceList[deviceIdx].name );
  if (status != sanepp::Status::Good)
  {
    std::cerr<<"Device "<<deviceList[deviceIdx].name<<" could not be accessed: "<<std::to_string(status)<<std::endl;
    return -1;
  }

  std::vector<sanepp::Option> options;
  status = dev.GetOptions( options );
  if (status != sanepp::Status::Good)
  {
    std::cerr<<"Failed to retrieve options"<<std::endl;
  }

  {
    for (int i=0; i<options.size(); ++i )
    {
      auto option = options[i];
      switch( option.type )
      {
      case sanepp::OptionValueType::Boolean:
        {
          bool boolval = 0;
          status = dev.GetOption( option.name, boolval);
        }
        break;
      case sanepp::OptionValueType::Integer:
        {
          int intval = 0;
          status = dev.GetOption( option.name, intval);
          if ( (status == sanepp::Status::Good) && (selectedOptions.count(option.name) > 0) )
          {
            if (sscanf(selectedOptions[option.name].c_str(), "%d", &intval) != 1)
            {
              std::cerr<<"Option "<<option.name<<" not an integer: "<<selectedOptions[option.name]<<std::endl;
              return -1;
	    }
            status = dev.SetOption( option.name, intval );
          }
        }
        break;
      case sanepp::OptionValueType::String:
        {
          std::string strval;
          status = dev.GetOption( option.name, strval);
          if ( (status == sanepp::Status::Good) && (selectedOptions.count(option.name) > 0) )
          {
            status = dev.SetOption( option.name, selectedOptions[option.name] );
	    if (status != sanepp::Status::Good)
	    {
              std::cerr<<"Failed to set option "<<option.name<<std::endl;
	      return -1;
	    }
          }
        }
        break;
      case sanepp::OptionValueType::Float:
        {
          double dval = 0.0;
          status = dev.GetOption( option.name, dval);
          if ( (status == sanepp::Status::Good) && (selectedOptions.count(option.name) > 0) )
          {
            float fval = 0.0;
            if (sscanf(selectedOptions[option.name].c_str(), "%f", &fval) != 1)
            {
              std::cerr<<"Option "<<option.name<<" not a floating point value: "<<selectedOptions[option.name]<<std::endl;
	      return -1;
            }
            dval = fval;
	    status = dev.SetOption( option.name, dval );
          }
        }
        break;
      }
    }
  }

  switch( op )
  {
  case Operation::DumpOptions:
    OutputOptions( dev, options );
  break;
  case Operation::Scan:
    status = dev.Start();
    if (status != sanepp::Status::Good)
    {
      std::cerr<<"Device "<<deviceList[deviceIdx].name<<" could not start scanning: "<<std::to_string(status)<<std::endl;
      return -1;
    }

    SANE_Parameters parm;
    status = dev.GetParameters( parm );
    if (status != sanepp::Status::Good)
    {
      std::cerr<<"Device "<<deviceList[deviceIdx].name<<" could not get parameters: "<<std::to_string(status)<<std::endl;
      return -1;
    }
    ColorSpace cs = ColorSpace::RGB;
    switch (parm.format)
    {
    case SANE_FRAME_RGB:
      cs = ColorSpace::RGB;
      break;
    case SANE_FRAME_GRAY:
      cs = ColorSpace::GRAY;
      break;
    }
    std::vector<uint8_t> data;
    while ( (status = dev.Read( data, 4096 )) != sanepp::Status::Eof)
    {
      if (status != sanepp::Status::Good)
      {
        std::cerr<<"Scanning interrupted."<<std::endl;
        return -1;
      }
    }
    pdf.AddPage();
    pdf.DrawImage( parm.pixels_per_line, parm.lines, cs, parm.depth, data );
    if (annotate)
    {
      char str[512];
      time_t t = time(NULL);
      struct tm* tm = localtime(&t);
      strftime(str, sizeof(str)-1, "Scanned: %c", tm);
      char* user = getenv("USER");
      if (user)
      {
        strcat( str, " - " );
        strcat( str, user );
      }
      std::cout<<"Drawing text: "<<str<<std::endl;
      pdf.DrawText( 0.06, 0, str );
    }
    pdf.Save( outputfile );
    break;
  }
}
