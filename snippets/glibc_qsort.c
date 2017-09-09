作者：丁冬
链接：https://www.zhihu.com/question/39214230/answer/80244880
来源：知乎
著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。

/*
首先实现qsort需要有一个swap：
*/

#define SWAP(a, b, size)
  do
    {
      size_t __size = (size);
      char *__a = (a), *__b = (b);
      do
	{
	  char __tmp = *__a;
	  *__a++ = *__b;
	  *__b++ = __tmp;
	} while (--__size > 0);
    } while (0)

/*
为了阅读方便，我把用于宏定义续行用的\去掉了，方便看到语法高亮。

SWAP的定义很简单，只是以char为单位长度进行交换。实际上，对于足够大的对象，这里还有
一定的优化空间，但qsort不能假定你的对象足够大。这也是qsort统计性能不如C++的
std::sort的原因之一。

所谓qsort并不是单纯的快速排序，当快速排序划分区间小到一定程度时，改用插入排序可以在
更少的耗时内完成排序。glibc将这个小区间定义为元素数不超过常数4的区间：
*/

#define MAX_THRESH 4

/*
可以认为对每一个小区间的插入排序耗时是不超过一个常数值（即对4个元素进行插入排序的最
差情况）的，这样插入排序总耗时也可以认为是线性的，不影响总体时间复杂度。

接下来这段不难理解：
*/

typedef struct
  {
    char *lo;
    char *hi;
  } stack_node;

#define STACK_SIZE	(CHAR_BIT * sizeof(size_t))
#define PUSH(low, high)	((void) ((top->lo = (low)), (top->hi = (high)), ++top))
#define	POP(low, high)	((void) (--top, (low = top->lo), (high = top->hi)))
#define	STACK_NOT_EMPTY	(stack < top)

/*
这显然是要手动维护栈来模拟递归，避免实际递归的函数调用开销了。题主自己的实现在元素数
量过多的时候会崩溃，估计就是递归过深爆栈了。

比较值得一提的是STACK_SIZE的选择。由于元素数量是由一个size_t表示的，size_t的最大
值应该是2 ^ (CHAR_BIT * sizeof(size_t))，快速排序的理想“递归”深度是对元素总数
求对数，所以理想的“递归”深度是CHAR_BIT * sizeof(size_t)。虽然理论上最差情况下
“递归”深度会变成2 ^ (CHAR_BIT * sizeof(size_t))，但是一方面我们不需要从头到尾
快速排序（区间足够小时改用插入排序），另一方面，我们是在假设元素数量等于size_t的上
限...这都把内存给挤满了=_=所以最后glibc决定直接采用理想“递归”深度作为栈大小上限。

接下来该进入qsort的正文了：
*/

void
_quicksort (void *const pbase, size_t total_elems, size_t size,
	    __compar_d_fn_t cmp, void *arg)
{

/*
qsort实际上是通过调用这个_quicksort实现的。最后的arg是个workaround，用来搞定
qsort_r的。暂且不去理会。下面是一些准备工作：
*/

  char *base_ptr = (char *) pbase;

  const size_t max_thresh = MAX_THRESH * size;

  if (total_elems == 0)
    return;

/*
明确了对内存操作是以sizeof(char)为单位的。max_thresh实际上是改用插入排序时，维护
当前区间的两个指针（类型均为char*）之间的距离。另外，如果元素数量为0，qsort啥也不
干，函数直接返回。
*/

  if (total_elems > MAX_THRESH)
 // 如果元素数大于终止区间（4个元素），则进行快速排序
    {
      char *lo = base_ptr;
      char *hi = &lo[size * (total_elems - 1)];
      stack_node stack[STACK_SIZE];
      stack_node *top = stack;

      PUSH (NULL, NULL);

/*
开始维护“递归”栈
*/

      while (STACK_NOT_EMPTY)
        {
          char *left_ptr;
          char *right_ptr;

/*
这里采用的是中值划分，用于降低特定序列导致递归恶化影响。也就是说，对区间里的首元
素、中间元素和尾元素先进行排序。排序方式是...呃，是冒泡。反正也就3个元素嘛。
*/

	  char *mid = lo + size * ((hi - lo) / size >> 1);

	  if ((*cmp) ((void *) mid, (void *) lo, arg) < 0)
	    SWAP (mid, lo, size);
	  if ((*cmp) ((void *) hi, (void *) mid, arg) < 0)
	    SWAP (mid, hi, size);
	  else
	    goto jump_over;
	  if ((*cmp) ((void *) mid, (void *) lo, arg) < 0)
	    SWAP (mid, lo, size);
	jump_over:;

/*
接下来就要进行快排划分了：
*/

	  left_ptr  = lo + size;
	  right_ptr = hi - size;

	  do
	    {
	      while ((*cmp) ((void *) left_ptr, (void *) mid, arg) < 0)
		left_ptr += size;

	      while ((*cmp) ((void *) mid, (void *) right_ptr, arg) < 0)
		right_ptr -= size;

/*
和传统的划分方式不同，首先把中间当作键值在左侧找到一个不小于mid的元素，右侧找到一个
不大于mid的元素
*/

	      if (left_ptr < right_ptr)
		{
		  SWAP (left_ptr, right_ptr, size);
		  if (mid == left_ptr)
		    mid = right_ptr;
		  else if (mid == right_ptr)
		    mid = left_ptr;
		  left_ptr += size;
		  right_ptr -= size;
		}

/*
如果左右侧找到的不是同一个元素，那就交换之。如果左右侧任意一侧已经达到mid，就把mid往
另一边挪。因为键值已经被“丢”过去了。
*/

	      else if (left_ptr == right_ptr)
		{
		  left_ptr += size;
		  right_ptr -= size;
		  break;
		}
	    }

/*
如果左右侧是同一元素，划分其实已经大功告成了。
*/

	  while (left_ptr <= right_ptr);

/*
接下来，将划分出来的所有大于的终止区间的区间压入栈准备下一次划分，连这里都会发生丧心
病狂的优化...不能浪费已经在栈里的原区间上下界=_=
*/

          if ((size_t) (right_ptr - lo) <= max_thresh)
            {
              if ((size_t) (hi - left_ptr) <= max_thresh)
                POP (lo, hi);
              else
                lo = left_ptr;
            }
          else if ((size_t) (hi - left_ptr) <= max_thresh)
            hi = right_ptr;
          else if ((right_ptr - lo) > (hi - left_ptr))
            {
              PUSH (lo, right_ptr);
              lo = left_ptr;
            }
          else
            {
              PUSH (left_ptr, hi);
              hi = right_ptr;
            }
        }
    }

/*
至此快速排序工作结束。
*/

#define min(x, y) ((x) < (y) ? (x) : (y))
 // 头一回学宏“函数”都应该见过这货吧

/*
接下来是插入排序
*/

  {

/*
众所周知，传统的插入排序中每一趟插入（假设是向前插入），我们都会迭代有序区间，如果已
经迭代到了区间头部，那就把该元素插在区间首部；否则如果前一个元素不大于待插入元素，则
插在该元素之后。

这种插入方式会导致每一次迭代要进行两次比较：一次比较当前迭代位置与区间首部，一次比较
两个元素大小。但事实上，我们可以用一个小技巧省掉前一次比较————插入排序开始前，先将容
器内最小元素放到容器首部，这样就可以保证每趟插入你永远不会迭代到区间首部，因为你总能
在中途找到一个不小于自己的元素。

注意，如果区间大于终止区间，搜索最小元素时我们不必遍历整个区间，因为快速排序保证最小
元素一定被划分到了第一个终止区间，也就是头4个元素之内。
*/

    char *const end_ptr = &base_ptr[size * (total_elems - 1)];
    char *tmp_ptr = base_ptr;
    char *thresh = min(end_ptr, base_ptr + max_thresh);
    char *run_ptr;

    for (run_ptr = tmp_ptr + size; run_ptr <= thresh; run_ptr += size)
      if ((*cmp) ((void *) run_ptr, (void *) tmp_ptr, arg) < 0)
        tmp_ptr = run_ptr;

    if (tmp_ptr != base_ptr)
      SWAP (tmp_ptr, base_ptr, size);

/*
准备工作完毕，这下可以放心地干掉迭代位置的比较了。
*/

    run_ptr = base_ptr + size;
    while ((run_ptr += size) <= end_ptr)
      {
	tmp_ptr = run_ptr - size;
	while ((*cmp) ((void *) run_ptr, (void *) tmp_ptr, arg) < 0)
	  tmp_ptr -= size;

	tmp_ptr += size;
        if (tmp_ptr != run_ptr)
          {
            char *trav;

	    trav = run_ptr + size;
	    while (--trav >= run_ptr)
              {
                char c = *trav;
                char *hi, *lo;

                for (hi = lo = trav; (lo -= size) >= tmp_ptr; hi = lo)
                  *hi = *lo;
                *hi = c;
              }
          }
      }
  }
}

/*
至此，qsort工作完成。
*/