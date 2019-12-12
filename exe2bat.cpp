#include <iostream>
#include <fstream>
#include <limits>

const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char* b64_encode( uint8_t* in, size_t len )
{
	size_t elen = ((len + 2) / 3 * 4) + 1;
	char* out = new char[elen];
	size_t  i;
	size_t  j;
	size_t  v;

	if( in == NULL || len == 0 ) return NULL;

	for (i=0, j=0; i<len; i+=3, j+=4) {
		v = in[i];
		v = i+1 < len ? v << 8 | in[i+1] : v << 8;
		v = i+2 < len ? v << 8 | in[i+2] : v << 8;

		out[j]   = b64chars[(v >> 18) & 0x3F];
		out[j+1] = b64chars[(v >> 12) & 0x3F];
		if (i+1 < len) out[j+2] = b64chars[(v >> 6) & 0x3F];
			else out[j+2] = '=';
		if (i+2 < len) out[j+3] = b64chars[v & 0x3F];
			else out[j+3] = '=';
	}
	
	return out;
}

using namespace std;

int main (int argc, char* argv[]) {
	
	string outfilename;
	string namever = "EXE2BAT 0.5";
	cout << namever << "\n\n";
	system("chcp 65001>nul");
	
	if( argc == 2 ) {
		if( string(argv[1]) == "?" ) {
			cout // THE MIT LICENSE
			<< "Copyright (c) 2019 Piotr Ambrożewicz\n\n"
			<< "Permission is hereby granted, free of charge, to any person obtaining\n"
			<< "a copy of this software and associated documentation files (the \"Software\"),\n"
			<< "to deal in the Software without restriction, including without limitation\n"
			<< "the rights to use, copy, modify, merge, publish, distribute, sublicense,\n"
			<< "and/or sell copies of the Software, and to permit persons to whom the Software\n"
			<< "is furnished to do so, subject to the following conditions:\n\n"

			<< "The above copyright notice and this permission notice shall be included\n"
			<< "in all copies or substantial portions of the Software.\n\n"

			<< "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS\n"
			<< "OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
			<< "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL\n"
			<< "THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES\n"
			<< "OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,\n"
			<< "ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE\n"
			<< "OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n";
			return 0;
		}
		else {
			outfilename = string(argv[1]) + ".bat";
		}
	}
	else if( argc != 3 ) //print usage
	{
		cerr
		<<"Copyright (c) 2019 Piotr Ambrożewicz"<<endl
		<<endl
		<<"Usage:\n"
		<<argv[0]<<" <input> <output>   - converts any file to self-extracting bat (w/ autorun)"<<endl
		<<argv[0]<<" <input>            - auto filename. This allows for drag&drop capabilities"<<endl
		<<argv[0]<<" ?                  - displays the license"<<endl;
		return 1;
	}
	else {
		outfilename = string(argv[2]);
	}
	
	fstream fin,fout;
	const uint16_t lenlim = 8190;
	
	fin.open( argv[1], ios::in | ios::binary );
	fin.ignore( numeric_limits<streamsize>::max() );
	streamsize finlen = fin.gcount();
	fin.clear();
	fin.seekg( 0, ios_base::beg );
	
	char* dat = new char[finlen];

	fin.read( dat, finlen );//read
	fin.close();
	
	uint32_t b64len = ((finlen + 2) / 3 * 4) + 1;
	char* b64 = new char[b64len];
	
	cout<<"input file length: "<<finlen<<endl;
	cout<<"encoded data length: "<<b64len<<endl;

	//convert to b64
	b64 = b64_encode( (uint8_t*)dat, finlen );
	
	delete dat;

	fout.open( outfilename, ios::out );
	fout.close();
	fout.open( outfilename, ios::out | ios::binary );
	if(fout.bad()) { cerr<<"Cannot open/write output file.\n"; return 1; }
	
	fout << "@echo off\r\n";
	fout << "echo " << namever << "\r\n";
	fout << "echo Extracting...\r\n";
	fout << ">t echo ";
	
	fout.write( b64, lenlim+1 ); //first segment
	fout << "\r\n";

	if ( b64len > lenlim ) {
		for( uint16_t i=0; i < b64len / lenlim -1; ++i ) {
			fout << ">>t echo ";
			fout.write( b64+(lenlim+1+i*lenlim), lenlim );
			fout << "\r\n";
		}
		cout<<"success!\n";
		if( b64len % lenlim ) {
			fout << ">>t echo ";
			fout.write( b64+(lenlim+1+(b64len / lenlim - 1)*lenlim), b64len % lenlim );
			fout << "\r\n\r\n";
		}
	}

	delete b64;
	fout << "echo Decoding...\r\n";
	fout << "certutil -f -decode t %temp%\\%~n0 >nul\r\n";
	fout << "del t\r\n";
	fout << "cls\r\n";
	fout << "%temp%\\%~n0 %*\r\n";
	fout << "del %temp%\\%~n0";
	
	return 0;
}
