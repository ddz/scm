(define add1
  (lambda (x)
    (+ x 1)))

(define sub1
  (lambda (x)
    (- x 1)))

(define alt
  (lambda (ls)
    (letrec
      ((alt-row
	(lambda (j k ls)
	  (if (null? ls)
	      ()
	      (cons (* j (* k (car ls)))
		    (alt-row j (* -1 k) (cdr ls))))))
       (alt-col
	(lambda (l m ls)
	  (if (null? ls)
	      ()
	      (cons (alt-row l m (car ls))
		    (alt-col (* -1 l) m (cdr ls)))))))
      (if (pair? (car ls))
	  (alt-col 1 1 ls)
	  (alt-row 1 1 ls)))))

(define sublist
  (lambda (ls n m)
    (letrec 
      ((tail
	(lambda (ls n)
	  (if (= n 0)
	      ls
	      (tail (cdr ls) (sub1 n)))))
       (head
	(lambda (ls m)
	  (if (= m 0)
	      ()
	      (cons (car ls) (head (cdr ls) (sub1 m)))))))
      (head (tail ls n) (- m n)))))

(define kill-row
  (lambda (A r)
    (append (sublist A 0 r)
	    (sublist A (add1 r) (length A)))))

(define kill-col
  (lambda (A c)
    (map (lambda (x) (kill-row x c)) A)))

(define iota
  (lambda (n)
    (letrec
      ((loop 
	(lambda (m acc)
	  (if (= m 0)
	      acc
	      (loop (sub1 m) (cons m acc))))))
      (loop n ()))))

(define determinant
  (lambda (A)
    (let ((n (length A)))
      (if (= n 1)
	  (car (car A))
	  (let* ((B (kill-row A 0))
	         (minors (map (lambda (x) (kill-col B (sub1 x))) (iota n)))
		 (cofactors (map determinant minors)))
	    (apply + (map * (alt (list-ref A 0)) cofactors)))))))

(define A '((1  2  3  4  5) 
            (6 -5  4 -3  2) 
            (1  3  6  2  4) 
            (3  4  5 -1 -3) 
	    (1  0 -1  0 -1)))

(determinant A)
