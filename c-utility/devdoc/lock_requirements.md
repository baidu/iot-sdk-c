# lock


## 简介

**lock**适配器实现了**threadapi**适配器中需要的同步的功能。

只有在**threadapi**适配器被实现且使用的时候才需要适配**lock**适配器，其他情况下该适配器不是必须的。

## 暴露的 API
**SRS_LOCK_10_001: [** `Lock` 接口暴露了以下 APIs **]**
```c
typedef enum LOCK_RESULT_TAG
{
    LOCK_OK,
    LOCK_ERROR
} LOCK_RESULT;
```

```c
typedef void* HANDLE_LOCK;
```
**SRS_LOCK_10_015: [** 这是对于不同的lock的句柄 **]**

```c
HANDLE_LOCK Lock_Init (void);
```
**SRS_LOCK_30_001: [** 如果**lock**没有被实现，`Lock_Init` 应该返回NULL。 **]**

**SRS_LOCK_10_002: [** `Lock_Init` 成功会返回一个合法且NULL的 lock 句柄。 **]**

**SRS_LOCK_10_003: [** `Lock_Init` 失败会返回 `NULL`。 **]**

```c
LOCK_RESULT Lock(HANDLE_LOCK handle);
```
**SRS_LOCK_30_001: [** 如果**lock**没有被实现，`Lock` 应该返回 `LOCK_ERROR`。 **]**

**SRS_LOCK_10_004: [** `Lock` 应该被实现成一个“非递归”的锁。 **]**

**SRS_LOCK_10_005: [** `Lock` 成功会返回 `LOCK_OK`。 **]**

**SRS_LOCK_10_006: [** `Lock` 失败会返回 `LOCK_ERROR`。 **]**

**SRS_LOCK_10_007: [** 在值为NULL的句柄上执行`Lock`，将会返回 `LOCK_ERROR`。 **]**

```c
LOCK_RESULT Unlock(HANDLE_LOCK handle);
```
**SRS_LOCK_30_001: [** 如果**lock**没有被实现，`Unlock` 应该返回 `LOCK_ERROR`。 **]**

**SRS_LOCK_10_008: [** `Unlock` 应该被实现成一个互斥的unlock。 **]**

**SRS_LOCK_10_009: [** `Unlock` 成功返回 `LOCK_OK`。 **]**

**SRS_LOCK_10_010: [** `Unlock` 失败返回 `LOCK_ERROR`。 **]**

**SRS_LOCK_10_011: [** 在值为NULL的句柄上执行`Unlock`，将会返回 `LOCK_ERROR`。 **]**

```c
LOCK_RESULT Lock_Deinit(HANDLE_LOCK handle);
```
**SRS_LOCK_30_001: [** 如果**lock**没有被实现，`Lock_Deinit` 应该返回 `LOCK_ERROR`。 **]**

**SRS_LOCK_10_012: [** `Lock_Deinit` 释放了 `handle`关联的所有资源。 **]**

**SRS_LOCK_10_013: [** 在值为NULL的句柄上执行`Lock_Deinit` 将会返回 `LOCK_ERROR` **]**
