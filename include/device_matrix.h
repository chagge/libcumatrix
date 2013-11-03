#ifndef __DEVICE_MATRIX_H__
#define __DEVICE_MATRIX_H__

#include <cassert>
#include <string>
using namespace std;

/* Includes, cuda */
#include <cuda_runtime.h>
#include <cublas_v2.h>
#include <helper_cuda.h>

#define CCE(x) checkCudaErrors(x)
#define STRIDE (sizeof(T) / sizeof(float))

template <typename T>
class SCALAR_MEMORY_BUFFER {
public:
  static T* getBuffer() {
    static SCALAR_MEMORY_BUFFER buffer;
    return buffer._ptr;
  }
private:
  SCALAR_MEMORY_BUFFER(): _ptr(NULL) {
    float scalar = 1;
    CCE(cudaMalloc((void **) &_ptr, sizeof(T)));
    CCE(cudaMemcpy(_ptr, &scalar, sizeof(T), cudaMemcpyHostToDevice));
  };
  SCALAR_MEMORY_BUFFER(const SCALAR_MEMORY_BUFFER& source);
  ~SCALAR_MEMORY_BUFFER() { CCE(cudaFree(_ptr)); }
  SCALAR_MEMORY_BUFFER& operator = (const SCALAR_MEMORY_BUFFER& rhs);

  T* _ptr;
};

class CUBLAS_HANDLE {
public:
  static cublasHandle_t& getInstance() {
    static CUBLAS_HANDLE H;
    return H._handle;
  }
private:
  CUBLAS_HANDLE()  { CCE(cublasCreate(&_handle)); }
  CUBLAS_HANDLE(const CUBLAS_HANDLE& source);
  ~CUBLAS_HANDLE() { CCE(cublasDestroy(_handle)); }
  CUBLAS_HANDLE& operator = (const CUBLAS_HANDLE& rhs);

  cublasHandle_t _handle;
};

template <typename T>
class device_matrix {
public:
  // default constructor 
  device_matrix();

  device_matrix(size_t r, size_t c);

  // Load from file. Ex: *.mat in text-form
  device_matrix(const string& filename);

  // Copy Constructor 
  device_matrix(const device_matrix<T>& source);

#ifdef HAVE_THRUST_DEVICE_VECTOR_H
  // Conversion operator
  operator thrust::device_vector<T>() const;
#endif

  // Destructor
  ~device_matrix();

  // ===========================
  // ===== Other Functions =====
  // ===========================
  
  // ===== Addition =====
  device_matrix<T>& operator += (T val);
  device_matrix<T> operator + (T val) const;
  
  device_matrix<T>& operator += (const device_matrix<T>& rhs);
  device_matrix<T> operator + (const device_matrix<T>& rhs) const;

  // ===== Substraction =====
  device_matrix<T>& operator -= (T val);
  device_matrix<T> operator - (T val) const;
  
  device_matrix<T>& operator -= (const device_matrix<T>& rhs);
  device_matrix<T> operator - (const device_matrix<T>& rhs) const;

  // ===== Division =====
  device_matrix<T>& operator /= (T alpha);
  device_matrix<T> operator / (T alpha) const;
  
  // ===== Matrix-scalar Multiplication =====
  device_matrix<T>& operator *= (T alpha);
  device_matrix<T> operator * (T alpha) const;

  // ===== Matrix-Matrix Multiplication =====
  device_matrix<T>& operator *= (const device_matrix<T>& rhs);
  device_matrix<T> operator * (const device_matrix<T>& rhs) const;

  template <typename S>
  friend void swap(device_matrix<S>& lhs, device_matrix<S>& rhs);

  // Operator Assignment:
  // call copy constructor first, and swap with the temp variable
  device_matrix<T>& operator = (device_matrix<T> rhs);

  void _init();
  void resize(size_t r, size_t c);
  void print(FILE* fid = stdout) const;

  void fillwith(T val);
  size_t size() const { return _rows * _cols; }
  size_t getRows() const { return _rows; }
  size_t getCols() const { return _cols; }
  T* getData() const { return _data; }
  void save(const string& filename) const;

  static void cublas_gemm(
    cublasOperation_t transA, cublasOperation_t transB,
    int m, int n, int k,
    T alpha,
    const T* A, int lda,
    const T* B, int ldb,
    T beta,
    T* C, int ldc);

  static void cublas_geam(
      cublasOperation_t transA, cublasOperation_t transB,
      int m, int n,
      T alpha, const T *A, int lda,
      T beta , const T *B, int ldb,
      T *C, int ldc);

  static void cublas_nrm2(int n, const T *x, int incx, T *result);

private:

  size_t _rows;
  size_t _cols;
  T* _data;
};

template <typename T>
void swap(device_matrix<T>& lhs, device_matrix<T>& rhs) {
  using std::swap;
  swap(lhs._rows, rhs._rows);
  swap(lhs._cols, rhs._cols);
  swap(lhs._data, rhs._data);
}

#define _DSMAT_ device_matrix<float>
void sgemm(const _DSMAT_& A, const _DSMAT_& B, _DSMAT_& C, float alpha = 1.0, float beta = 0.0);
void sgeam(const _DSMAT_& A, const _DSMAT_& B, _DSMAT_& C, float alpha = 1.0, float beta = 1.0);
float snrm2(const _DSMAT_& A);
#undef _DSMAT_


template <typename T, typename U>
device_matrix<T> operator + (U alpha, const device_matrix<T>& m) {
  return m + (T) alpha;
}

template <typename T, typename U>
device_matrix<T> operator - (U alpha, const device_matrix<T>& m) {
  return m - (T) alpha;
}

template <typename T, typename U>
device_matrix<T> operator * (U alpha, const device_matrix<T>& m) {
  return m * (T) alpha;
}

#endif // __DEVICE_MATRIX_H__