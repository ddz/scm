(define null?
  (lambda (ls)
    (= ls '())))

(define length
  (lambda (ls)
    (if (null? ls) 0 (+ 1 (length (cdr ls))))))

(define append
  (lambda (ls1 ls2)
    (if (null? ls1) ls2 (cons (car ls1) (append (cdr ls1) ls2)))))

(define map
  (lambda (f ls)
    (if (null? ls)
	ls
	(cons (f (car ls)) (map f (cdr ls))))))

(define map2
  (lambda (f ls1 ls2)
    (if (null? ls1)
	'()
	(cons (f (car ls1) (car ls2)) (map2 f (cdr ls1) (cdr ls2))))))


(define list-ref
  (lambda (ls n)
    (if (= n 0) (car ls) (list-ref (cdr ls) (- n 1)))))

(define add1
  (lambda (x)
    (+ x 1)))

(define sub1
  (lambda (x)
    (- x 1)))

(define alt
  (lambda (ls)
    (let ((alt-row 1) (alt-col 1))
      (set! alt-row
	    (lambda (j k ls)
	      (if (null? ls)
		  ()
		  (cons (* j (* k (car ls)))
			(alt-row j (* -1 k) (cdr ls))))))
      (set! alt-col
	    (lambda (l m ls)
	      (if (null? ls)
		  ()
		  (cons (alt-row l m (car ls))
			(alt-col (* -1 l) m (cdr ls))))))
      (if (pair? (car ls))
	  (alt-col 1 1 ls)
	  (alt-row 1 1 ls)))))

(let ((foo 1) (bar 2))
  (set! foo
	(lambda (n)
	  (if (= n 0) n (foo (- n 1)))))
  (set! bar
	(lambda (n)
	  (if (= n 0) n (bar (- n 1)))))
  (foo 2))

(define sublist
  (lambda (ls n m)
    (let ((tail 1) (head 1))
      (set! tail
	    (lambda (ls n)
	      (if (= n 0)
		  ls
		  (tail (cdr ls) (sub1 n)))))
       (set! head
	     (lambda (ls m)
	       (if (= m 0)
		   ()
		   (cons (car ls) (head (cdr ls) (sub1 m))))))
       (head (tail ls n) (- m n)))))

(define kill-row
  (lambda (a r)
    (append (sublist a 0 r)
	    (sublist a (add1 r) (length a)))))

(define kill-col
  (lambda (a c)
    (map (lambda (x) (kill-row x c)) a)))

(define iota
  (lambda (n)
    (let ((loop 1))
      (set! loop 
	    (lambda (m acc)
	      (if (= m 0)
		  acc
		  (loop (sub1 m) (cons m acc)))))
      (loop n ()))))

(define determinant
  (lambda (a)
    (let ((n (length a)))
      (if (= n 1)
	  (car (car a))
	  (let ((B (kill-row a 0)))
	    (let ((minors (map (lambda (x) (kill-col B (sub1 x))) (iota n))))
		 (let ((cofactors (map determinant minors)))
		   (apply + (map2 * (alt (list-ref a 0)) cofactors)))))))))

(define a '((1  2  3  4  5) 
            (6 -5  4 -3  2) 
            (1  3  6  2  4) 
            (3  4  5 -1 -3) 
	    (1  0 -1  0 -1)))

(define b '((1 2) (3 4)))

(determinant a)
(determinant b)