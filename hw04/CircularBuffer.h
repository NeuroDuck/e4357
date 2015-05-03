#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stdint.h>

/** This class implements a static circular buffer.
*/
template<size_t T>
class CircularBuffer
{
    public :
    
        /** Default constructor
        */
        CircularBuffer();
        
        /** Reads data from buffer
        
            \param data output buffer 
            \param length Maximum number of bytes to read
            \return Number of bytes read
            
            \note The return value cannot exceed max(length,capacity)
        */
        uint32_t read(uint8_t *data, uint32_t length);
        
        /** Writes data in buffer
        
            \param data input buffer
            \param length Maximum number of bytes to write
            \return Number of bytes wrote
            
            \note The return value cannot exceed max(length,capacity)
        */
        uint32_t write(uint8_t *data, uint32_t length);
        
        /** Returns the total capacity of this buffer
            \return Capacity of buffer
        */
        uint32_t getCapacity() const;
        
        /** Returns the number of bytes available in the buffer
            \return Number of bytes available in the buffer
        */
        uint32_t getSize() const;  
        
        /** Checks if this buffer is empty
            \return True if the buffer is empty, false otherwise
        */
        bool isEmpty() const;
        
        /** Checks if this buffer is full
            \return True if the buffer is full, false otherwise
        */        
        bool isFull() const;
        
    private :
    
        uint16_t readIndex, writeIndex;
        uint8_t buffer[T]; 
        size_t bytesAvailable;
};

template<size_t T>
CircularBuffer<T>::CircularBuffer():
readIndex(0),
writeIndex(0),
bytesAvailable(0)
{
}

template<size_t T>
uint32_t CircularBuffer<T>::read(uint8_t *data, uint32_t length)
{
    uint32_t n = 0;
    while(n < length && getSize() > 0)
    {
        data[n++] = buffer[readIndex++];
        if(readIndex == T)
            readIndex = 0;
        --bytesAvailable;
    }
    
    return n;
}

template<size_t T>
uint32_t CircularBuffer<T>::write(uint8_t *data, uint32_t length)
{
    uint32_t n = 0;
    while(n < length && getSize() < T)
    {
        buffer[writeIndex++] = data[n++];
        if(writeIndex == T)
            writeIndex = 0;
        ++bytesAvailable;
    }
        
    return n;
}

template<size_t T>
uint32_t CircularBuffer<T>::getCapacity() const
{
    return T;
}
        
template<size_t T>
inline uint32_t CircularBuffer<T>::getSize() const
{
    return bytesAvailable;
}

template<size_t T>
inline bool CircularBuffer<T>::isEmpty() const
{
    return getSize() == 0;
}

template<size_t T>
inline bool CircularBuffer<T>::isFull() const
{
    return getSize() == T;
}

typedef CircularBuffer<32> SmallCircularBuffer;
typedef CircularBuffer<128> MediumCircularBuffer;
typedef CircularBuffer<512> BigCircularBuffer;

#endif
