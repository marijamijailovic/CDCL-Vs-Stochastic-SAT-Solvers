#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include <string>
#include <cstdlib>
#include <sys/param.h>
#include <unistd.h>
#include <cmath>
#include <stdexcept>

using namespace std;

#define ASSERT(condition, statement) \
    do { \
        if (!(condition)) { \
           statement; assert(condition); \
        } \
    } while (false)

using Sudoku = vector<vector<unsigned>>;
using Atom = unsigned;
using Clause = vector<int>;


/*  We use Xi,j,k  where i is index of row,
 *  j is index of column, and k is value assigned to this field*/
void generateFormula(Sudoku sudoku,unsigned n)
{
  Atom atom = n*n*n;
  vector<Clause> clauses;
  ofstream cnfFile("dimacs.cnf");
  ASSERT(cnfFile, std::cerr << "File can't be open for writing" << endl);

  for(unsigned i = 0; i < n; i++){
    for(unsigned j = 0; j < n ; j++){
      cnfFile << "c";
      for(unsigned k = 1; k <= n; k++){
        cnfFile << " x_" << i << "," << j << "," << k;
        cnfFile << "-->" << (i*n*n+j*n+k) << "\t";
      }
      cnfFile << endl;
    }
  }


  /*  1) At least one number in each entry from (1 to n)
   *  Xi,j,1 \/ Xi,j,2 \/ ... Xi,j,n */
  for(unsigned i = 0; i < n; i++){
    for(unsigned j = 0; j < n; j++){
      Clause c;
      for(unsigned k = 1; k <= n; k++){
        c.push_back(i*n*n+j*n+k);
      }
      clauses.push_back(c);
    }
  }

  /*  2) At most one value in each filed
   *  ~(Xi,j,k1 /\ Xi,j,k2) --> ~Xi,j,k1 \/ ~Xi,j,k2*/
  for(unsigned i = 0; i < n; i++){
    for(unsigned j = 0; j < n; j++){
      for(unsigned k = 1; k <= n; k++){
        for(unsigned k_s = k+1; k_s <= n; k_s++){
          Clause c;
          c.push_back(-(i*n*n+j*n+k));
          c.push_back(-(i*n*n+j*n+k_s));
          clauses.push_back(c);
        }
      }
    }
  }

  /*  3) Each number at most once in row
   *  ~(Xi,j1,k /\ Xi,j2,k) --> ~Xi,j1,k \/ ~Xi,j2,k2 */
  for(unsigned i = 0; i < n; i++){
    for(unsigned j = 0; j < n; j++){
      for(unsigned k = 1; k <= n; k++){
        for(unsigned j_s = j+1; j_s < n; j_s++){
          Clause c;
          c.push_back(-(i*n*n+j*n+k));
          c.push_back(-(i*n*n+j_s*n+k));
          clauses.push_back(c);
        }
      }
    }
  }

  /*  4) Each number at most once in column
  *  ~(Xi1,j,k /\ Xi2,j,k) --> ~Xi1,j,k \/ ~Xi2,j,k */
  for(unsigned i = 0; i < n; i++){
    for(unsigned j = 0; j < n; j++){
      for(unsigned k = 1; k <= n; k++){
        for(unsigned i_s = i+1; i_s < n; i_s++){
          Clause c;
          c.push_back(-(i*n*n+j*n+k));
          c.push_back(-(i_s*n*n+j*n+k));
          clauses.push_back(c);
        }
      }
    }
  }

  /*  5) Each number at most once in each block
   *
   *  ~(Xsqrt(n)*i+is,sqrt(n)*j+js,k /\ Xsqrt(n)*i+is,sqrt(n)*j+jss,k)
   *  --> ~Xsqrt(n)*i+is,sqrt(n)*j+js,k \/ ~Xsqrt(n)*i+is,sqrt(n)*j+jss,k
   *  Each number at most once in row in same block*/

   unsigned sqrtn = (unsigned)sqrt(n);
   for(unsigned i = 0; i < sqrtn; i++){
     for(unsigned j = 0; j < sqrtn; j++){
       for(unsigned k = 1; k <= n; k++){
         for(unsigned i_s = 0; i_s < sqrtn; i_s++){
          for(unsigned j_s = 0; j_s < sqrtn; j_s++){
            for(unsigned j_ss = j_s + 1; j_ss < sqrtn; j_ss++){
              Clause c;
              c.push_back(-((i*sqrtn+i_s)*n*n+(j*sqrtn+j_s)*n+k));
              c.push_back(-((i*sqrtn+i_s)*n*n+(j*sqrtn+j_ss)*n+k));
              clauses.push_back(c);
            }
          }
         }
       }
     }
   }

   /* ~(Xsqrt(n)*i+is,sqrt(n)*j+js,k /\ Xsqrt(n)*i+iss,sqrt(n)*j+jss,k)
    *  --> ~Xsqrt(n)*i+is,sqrt(n)*j+js,k \/ ~Xsqrt(n)*i+iss,sqrt(n)*j+jss,k
    *  Each number at most once in column and each number at most once diagonal in same block*/

   for(unsigned i = 0; i < sqrtn; i++){
     for(unsigned j = 0; j < sqrtn; j++){
       for(unsigned k = 1; k <= n; k++){
         for(unsigned i_s = 0; i_s < sqrtn; i_s++){
          for(unsigned j_s = 0; j_s < sqrtn; j_s++){
            for(unsigned i_ss = i_s + 1; i_ss < sqrtn; i_ss++){
              for(unsigned j_ss = 0; j_ss < sqrtn; j_ss++){
                Clause c;
                c.push_back(-((i*sqrtn+i_s)*n*n+(j*sqrtn+j_s)*n+k));
                c.push_back(-((i*sqrtn+i_ss)*n*n+(j*sqrtn+j_ss)*n+k));
                clauses.push_back(c);
              }
            }
          }
         }
       }
     }
   }

  /*  6) Prefilled value*/
  for(unsigned i = 0; i < n; i++){
    for(unsigned j = 0; j < n; j++){
        if(sudoku[i][j] != 0){
          Clause c;
          c.push_back(i*n*n+j*n+sudoku[i][j]);
          clauses.push_back(c);
        }
    }
  }


  cnfFile << "p cnf " << atom << " " << clauses.size() << endl;
  for(auto clause : clauses){
    for(auto c : clause){
      cnfFile << c << " ";
    }
    cnfFile << "0" << endl;
  }

  cnfFile.close();
}

Sudoku readSudoku(unsigned n, string level)
{
  string nameOfFile = string("examples/") + level + string("_") + to_string(n) + string(".txt");

  ifstream sudokuFile(nameOfFile);
  ASSERT(sudokuFile, std::cerr << "File " << nameOfFile << " doesn't exist" << endl);

  Sudoku sudoku;
  for(unsigned i = 0; i < n; i++){
      sudoku.push_back(vector<unsigned>(n));
      for(unsigned j = 0; j < n; j++){
        sudokuFile >> sudoku[i][j];
      }
  }

  sudokuFile.close();

  return sudoku;
}

Sudoku generateSudoku(unsigned n)
{
  ifstream sudokuFile("sudoku.out");
  ASSERT(sudokuFile, std::cerr << "File sudoku.out doesn't exist" << endl);

  string s;
  sudokuFile >> s;
  Sudoku sudoku(n,vector<unsigned>(n));

  if(s == "SAT"){
    int value;
    sudokuFile >> value;
    vector<int> values;
    while(value != 0){
      values.push_back(value);
      sudokuFile >> value;
    }

    for(unsigned i = 0; i < n; i++){
      for(unsigned j = 0; j < n; j++){
        for(unsigned k = 0; k < n; k++){
          if(values[i*n*n+j*n+k] > 0){
            sudoku[i][j] = k+1;
            }
        }
      }
    }
  }
  else{
    throw runtime_error("UNSATISFIABLE");
  }

  sudokuFile.close();

  return sudoku;
}

int main(int argc, char** argv)
{
  ASSERT(argc == 3, cerr << "Usage: " << argv[0] << " dimension(4x4,9x9,16x16) " << " level(easy,medium,hard)" << endl);

  unsigned n = stoi(argv[1]);
  string level = argv[2];

  Sudoku sudoku;
  sudoku = readSudoku(n,level);
  generateFormula(sudoku,n);

  system("minisat dimacs.cnf sudoku.out");

  system("~/ubcsat/ubcsat-beta-12-b18/ubcsat -alg saps -i dimacs.cnf -solve");

  sudoku = generateSudoku(n);

  for(unsigned i=0;i<n;i++){
    for(unsigned j=0;j<n;j++){
      cout << sudoku[i][j] << " ";
    }
    cout << endl;
  }

  return 0;
}
