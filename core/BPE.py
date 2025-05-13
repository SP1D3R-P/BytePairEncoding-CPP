
try :
    import _core
except : 
    raise ImportError("_core is not found. Compile the cpp files to get the library")

from typing import Callable , Any , List , Dict , Optional , Literal , Type , get_args
from typing_extensions import Self
import beartype

import os
from pathlib import Path

import json
import base64

from hashlib import sha256

import numpy as np 

Pattern : Type  = Literal['GPT4','CLK50k']

@beartype.beartype
class BytePairEncoding(_core.pyBytePairEncoding) : 

    def __init__(self : Self,
                vocab_size : Optional[int]  = 0x101 ,
                pattern : Optional[Pattern | str ] = 'GPT4' ,
                special_tokens : Optional[str | set[str]]  = 'all'):
        """BytePairEncoding

        Args:
            self (Self): self
            vocab_size Optional(int): vocabulary size . Defaults to 0x101 aka 257.
            pattern Optional(str, Pattern): pattern to use [can be of given option or valid regex pattern]. Defaults to 'GPT4'.
            special_tokens Optional(str | set[str]): valid special token ['all',{'the set of token you want'}]. Defaults to 'all'.
        """
        if  vocab_size < 0x100 : 
            raise ValueError("The size of Vocab must be greater than 256. Passed {}".format(vocab_size))

        
        if not pattern in get_args(Pattern):
            raise NotImplementedError("Not Implemented.")
        else : 
            super().__init__(vocab_size)


    @property
    def vocab_size(self) -> int :
        """gets the vocab size of the tokenizer

        Returns:
            int: vocab size 
        """
        return super()._size 

    @property
    def pattern(self) -> str :
        """currently using pattern

        Returns:
            str: pattern used 
        """
        return super()._pattern

    @property
    def vocab_capacity(self : Self) -> int : 
        """capacity of tokens it should iterate upto

        Returns:
            int: total vocab cap
        """
        return super()._capacity

    @vocab_capacity.setter
    def vocab_capacity(self : Self, n : int ) : 
        """update the capcity of the token

        Args:
            n: new size
        """

        if( n < self.vocab_capacity ) : 
            raise ValueError("New Capacity[{new}] can't be less than Old Capacity[{old}]".format(new=n,old=self.vocab_capacity))

        self._capacity = n
            

    def train(self : Self) -> None:
        """train the tokenizer based on the Encoding 
        """
        super()._train()

    def encode(self : Self , text : str ) -> np.ndarray : 
        """encode

        Args:
            text (str): text 

        Raises:
            ValueError : If The size of the String is less than 2 
            
        Returns:
            np.ndarray: array of encoded text 
        """
        if len(text) < 2 : 
            raise ValueError("Expected size of string more than 2 but got {size}".format(size=len(text)))
        return super()._encode(text)

    def decode(self: Self , array : np.ndarray ) -> bytes : 
        """decode

        Args:
            array (np.ndarray): array like encoded string

        Raises:
            ValueError: Invalid Token 

        Returns:
            bytes: decoded string 
        """
        return super()._decode(array)

    @beartype.beartype
    def compile(self, f_name : str ) -> None :
        """compiles the file string to vector for further processing 

        Args:
            f_name (str): file name 
        """
        with open(f_name,'r') as fp : 
            super()._compile(fp.read(),f_name)



    @beartype.beartype
    def save(self,f_name : str ) -> None : 
        """saves the model 

        Raises:
            ValueError: Invalid File Type 
            NotADirectoryError: Invalid Directory 

        Args:
            f_name (str): file name 
        """

        f_type = f_name.split(".")[-1]

        match( f_type ) : 
            case "bpe" :
                f_type = 0
            case "json" :
                f_type = 1
            case _ : 
                raise ValueError("Invalid File type {} Passed {}".format(f_type,f_name))
        
        
        path : Path = Path(f_name)
        if not os.path.exists(path.parent) : 
            raise NotADirectoryError("No such Directory Found {}".format(path))

        path.touch()

        if f_type == 1 : 
            dictData : dict[int,bytes] = super()._table()
            writeData : dict = {}
            for key , value in dictData.items() : 
                writeData[base64.b64encode(value).decode()] = key 
            json_data : str  = json.dumps(writeData,indent=8)
            json_hash = sha256(json_data.encode()).hexdigest()

            data_to_write : str  = f"""
{{
    "id" : "{json_hash}",
    "data" : {json_data}
}}
"""

            path.write_text(data_to_write)
            

                
        else : 
            raise NotImplementedError("Not Implemeted for bpe")





        @classmethod
        def load(cls : BytePairEncoding , file_name : str  ) -> BytePairEncoding : 
            """load

            Args:
                cls (BytePairEncoding): ...
                file_name (str): file to load 

            Returns:
                BytePairEncoding: constructed  object 
            """
            ...
            
    

    

    

    