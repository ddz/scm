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
  (lambda (a r)
    (append (sublist a 0 r)
	    (sublist a (add1 r) (length a)))))

(define kill-col
  (lambda (a c)
    (map (lambda (x) (kill-row x c)) a)))

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
  (lambda (a)
    (let ((n (length a)))
      (if (= n 1)
	  (car (car a))
	  (let* ((B (kill-row a 0))
	         (minors (map (lambda (x) (kill-col B (sub1 x))) (iota n)))
		 (cofactors (map determinant minors)))
	    (apply + (map * (alt (list-ref a 0)) cofactors)))))))

(define a '((1  2  3  4  5) 
            (6 -5  4 -3  2) 
            (1  3  6  2  4) 
            (3  4  5 -1 -3) 
	    (1  0 -1  0 -1)))

(determinant a)
