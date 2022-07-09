-- Patch up the info, so after copyInfo.py

UPDATE description SET columnName='id',type='hash' WHERE sortKey=0;
UPDATE description SET columnName='displayName',type='h1' WHERE sortKey=1;
