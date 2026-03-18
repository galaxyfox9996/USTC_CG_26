// implementation of class DArray
#include "DArray.h"
#include <iostream>
#include <cassert>
using namespace std;
// default constructor
DArray::DArray() {
	Init();
}

// set an array with default values
DArray::DArray(int nSize, double dValue) {
	//TODO
	if(nSize < 0) {
		nSize = 0;
	}
	m_pData = new double[nSize];
	m_nSize = nSize;
	for(int i = 0;i<m_nSize;i++){
		m_pData[i] = dValue;
	}
}

DArray::DArray(const DArray& arr) {
	//TODO
	m_nSize = arr.m_nSize;
	if(m_nSize > 0 && arr.m_pData != nullptr){
		m_pData = new double[m_nSize];
		for(int i = 0;i < m_nSize;i++){
			m_pData[i] = arr.m_pData[i];
		}
	}
}

// deconstructor
DArray::~DArray() {
	Free();
}

// display the elements of the array
void DArray::Print() const {
	//TODO
	cout << "DArray: 大小 = " << m_nSize <<", 元素 :";
	for(int i =0;i < m_nSize ; i++){
		cout <<" "<<GetAt(i);
	}
	cout << endl;
}

// initilize the array
void DArray::Init() {
	//TODO
	m_nSize = 0;
	m_pData = nullptr;
}

// free the array
void DArray::Free() {
	//TODO
	if(m_pData != nullptr){
		delete[] m_pData;
		m_pData = nullptr;
	}
	m_nSize = 0;
}

// get the size of the array
int DArray::GetSize() const {
	//TODO
	return m_nSize; // you should return a correct value
}

// set the size of the array
void DArray::SetSize(int nSize) {
	//TODO
	if(m_nSize == nSize){
		return; // 大小不变，直接返回
	}
	double* pData = new double[nSize]; // 分配新内存
	int copyNum = (nSize < m_nSize) ? nSize : m_nSize;
	for(int i = 0;i < copyNum;i++){
		pData[i] = m_pData[i]; // 复制旧数据
	}
	for(int i = copyNum;i < nSize;i++){
		pData[i] = 0; // 初始化新元素
	}
	delete[] m_pData; // 释放旧内存
	m_pData = pData; // 更新指针
	m_nSize = nSize; // 更新大小
}

// get an element at an index
const double& DArray::GetAt(int nIndex) const {
	//TODO
	assert(nIndex >= 0 && nIndex < m_nSize);
	return m_pData[nIndex]; // you should return a correct value
}

// set the value of an element 
void DArray::SetAt(int nIndex, double dValue) {
	//TODO
	assert(nIndex >= 0 && nIndex < m_nSize);
	m_pData[nIndex] = dValue;
}

// overload operator '[]'
const double& DArray::operator[](int nIndex) const {
	//TODO
	assert(nIndex >= 0 && nIndex < m_nSize);
	return m_pData[nIndex]; // you should return a correct value
}

// add a new element at the end of the array
void DArray::PushBack(double dValue) {
	//TODO
	int newSize = (m_nSize == 0) ? 1 : m_nSize * 2; // 如果当前大小为0，则新大小为1，否则翻倍
	double* newData = new double[newSize]; // 分配新内存
	for(int i = 0;i<m_nSize;i++){
		newData[i] = m_pData[i]; // 复制旧数据
	}
	newData[m_nSize] = dValue; // 添加新元素
	delete[] m_pData; // 释放旧内存
	m_pData = newData; // 更新指针
	m_nSize++; // 更新大小
}

// delete an element at some index
void DArray::DeleteAt(int nIndex) {
	//TODO
	if(nIndex < 0 || nIndex >= m_nSize){
		return; // 索引无效，直接返回
	}
	for(int i = nIndex; i< m_nSize-1;i++){
		m_pData[i] = m_pData[i+1];
	}
	m_nSize--; // 更新大小
}

// insert a new element at some index
void DArray::InsertAt(int nIndex, double dValue) {
	//TODO
	if(nIndex < 0 || nIndex > m_nSize){
		return; // 索引无效，直接返回
	}
	int newSize = (m_nSize == 0) ? 1 : m_nSize * 2; // 如果当前大小为0，则新大小为1，否则翻倍
	double* newData = new double[newSize]; // 分配新内存
	for(int i = 0;i< nIndex;i++){
		newData[i] = m_pData[i]; // 复制前半部分数据
	}
	for(int i = nIndex;i<m_nSize;i++){
		newData[i+1] = m_pData[i]; // 复制后半部分数据
	}
	newData[nIndex] = dValue;
	delete[] m_pData; // 释放旧内存
	m_pData = newData; // 更新指针
	m_nSize++; // 更新大小
}

// overload operator '='
DArray& DArray::operator = (const DArray& arr) {
	//TODO
	if(this == &arr){
		return *this; // 自赋值，直接返回
	}
	Free(); // 释放旧内存
	m_nSize = arr.m_nSize; // 更新大小
	if(m_nSize > 0){
		m_pData = new double[m_nSize]; // 分配新内存
		for(int i = 0;i < m_nSize;i++){
			m_pData[i] = arr.m_pData[i]; // 复制数据
		}
	}

	return *this;
}
