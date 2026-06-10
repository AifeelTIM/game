#ifndef __JUSTFLOAT_H__
#define __JUSTFLOAT_H__

#include "stdint.h"

// 数据类型定义
typedef enum {
    JUSTFLOAT_TYPE_INT = 0,
    JUSTFLOAT_TYPE_FLOAT = 1
} JustFloat_DataType_t;

// 初始化函数（预留，当前不做操作）
void JustFloat_Init(void);

// 发送函数
// data_ptr: 指向int数组或float数组
// len: 数据长度（元素个数）
// type: 数据类型，JUSTFLOAT_TYPE_INT 或 JUSTFLOAT_TYPE_FLOAT
void JustFloat_Send(void *data_ptr, uint16_t len, JustFloat_DataType_t type);

#endif // __JUSTFLOAT_H__
