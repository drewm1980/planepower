#ifndef COUT_HPP
#define COUT_HPP

#ifdef ORO_OCL_HPP
#define COUT log(Info)
#define CERR log(Error)
#define ENDL endlog()
#else
#define COUT cout
#define CERR cerr
#define ENDL endl
#endif

#endif
