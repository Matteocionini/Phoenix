#include "moveArray.h"

void moveArray::pushBack(const uint32_t& val) {
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

uint32_t* moveArray::Begin() {
	return m_array;
}

uint32_t* moveArray::End() {
	return m_array + m_size;
}

void moveArray::Append(uint32_t* begin, uint32_t* end) {
	uint32_t* elem;

	for (elem = begin; elem < end; elem++) {
		pushBack(*elem);
	}
}

moveArray::moveArray() {
	m_size = 0;
}

uint32_t& moveArray::getElem(int index) {
	return m_array[index];
}

void  moveArray::reverse() {
	uint32_t* l = m_array;
	uint32_t* r = m_array + m_size;
	uint32_t temp;

	while (r > l) {
		temp = *l;
		*l = *r;
		*r = temp;

		l++;
		r--;
	}
}