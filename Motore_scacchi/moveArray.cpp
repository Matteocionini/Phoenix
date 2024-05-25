#include "moveArray.h"

void moveArray::pushBack(const uint16_t& val) {
	m_array[m_size] = val;
	++m_size;
}

/*uint16_t& moveArray::operator [](const int& index) {
	return m_array[index];
}*/

void moveArray::Reset() {
	m_size = 0;
}

const int& moveArray::getSize() const {
	return m_size;
}

uint16_t* moveArray::Begin() {
	return m_array;
}

uint16_t* moveArray::End() {
	return m_array + m_size;
}

void moveArray::Append(uint16_t* begin, uint16_t* end) {
	uint16_t* elem;

	for (elem = begin; elem < end; elem++) {
		pushBack(*elem);
	}
}

moveArray::moveArray() {
	m_size = 0;
}

uint16_t& moveArray::getElem(int index) {
	return m_array[index];
}